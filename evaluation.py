import re

def normalize_text(text: str) -> str:
    """Normalize text to compare: lowercase, collapse spaces."""
    return re.sub(r'\s+', ' ', (text or "").lower().strip())

def evaluate_search(user_answer, expected):
    user_norm = normalize_text(user_answer)
    expected_strategy = normalize_text(expected.get('strategy', ''))

    score = 0
    feedback = []

    # Strategy presence
    if expected_strategy and expected_strategy in user_norm:
        score += 60
        feedback.append("✓ Correct strategy identified")
    else:
        feedback.append("✗ Strategy incorrect or not identified")

    # Justification keywords (English/Spanish mixed to be tolerant)
    keywords = ['constraint', 'prune', 'backtrack', 'systematic', 'valid',
                'restriccion', 'poda', 'consistencia', 'dominio', 'csp']
    found_keywords = sum(1 for kw in keywords if kw in user_norm)

    if found_keywords >= 2:
        score += 40
        feedback.append("✓ Adequate justification")
    elif found_keywords == 1:
        score += 20
        feedback.append("⚠ Partial justification")
    else:
        feedback.append("✗ Missing justification")

    return min(score, 100), feedback

def evaluate_game_theory(user_answer, expected):
    user_norm = normalize_text(user_answer)
    score = 0
    feedback = []

    exists = expected.get('exists', False)
    says_yes = any(w in user_norm for w in ['yes', 'si', 'sí', 'exists', 'existe', 'there is'])
    says_no = any(w in user_norm for w in ['no', 'no existe', 'not exist', "doesn't exist", 'does not exist'])

    if (exists and says_yes) or (not exists and says_no):
        score += 40
        feedback.append("✓ Correct existence statement")
    else:
        feedback.append("✗ Incorrect existence statement")

    if exists:
        equilibrium = normalize_text(expected.get('equilibrium', ''))
        # Compare coordinates ignoring punctuation
        user_flat = user_norm.replace(',', '').replace('(', '').replace(')', '')
        eq_flat = equilibrium.replace(',', '').replace('(', '').replace(')', '')
        if eq_flat and eq_flat in user_flat:
            score += 30
            feedback.append("✓ Correct equilibrium profile")
        else:
            feedback.append("✗ Equilibrium profile missing or incorrect")

    justification_keywords = ['deviate', 'improve', 'incentive', 'best response',
                              'mejor', 'cambiar', 'unilateral', 'mejores respuestas']
    found = sum(1 for kw in justification_keywords if kw in user_norm)
    if found >= 2:
        score += 30
        feedback.append("✓ Sufficient justification")
    elif found == 1:
        score += 15
        feedback.append("⚠ Partial justification")
    else:
        feedback.append("✗ Missing justification")

    return min(score, 100), feedback

def evaluate_csp(user_answer, expected):
    user_norm = normalize_text(user_answer)
    expected_assignment = normalize_text(expected.get('assignment', ''))

    score = 0
    feedback = []

    # Extract pairs like x=1
    expected_pairs = re.findall(r'([a-z])=(\\d+)', expected_assignment)
    user_pairs = re.findall(r'([a-z])=(\\d+)', user_norm)

    expected_dict = {var: val for var, val in expected_pairs}
    user_dict = {var: val for var, val in user_pairs}

    correct = 0
    total = len(expected_dict)

    for var, val in expected_dict.items():
        if user_dict.get(var) == val:
            correct += 1
            feedback.append(f"✓ {var.upper()}={val} correct")
        else:
            feedback.append(f"✗ {var.upper()} incorrect (expected: {val}, got: {user_dict.get(var, 'N/A')})")

    score = int((correct / total) * 70) if total > 0 else 0

    # Bonus for process explanation
    if any(word in user_norm for word in ['forward checking', 'domain', 'reduce', 'constraint',
                                          'mrv', 'ac-3', 'consistency', 'propagation',
                                          'verificacion anticipada', 'dominio', 'consistencia']):
        score += 30
        feedback.append("✓ Process explanation included")
    else:
        feedback.append("⚠ Missing process explanation")

    return min(score, 100), feedback

def evaluate_minimax(user_answer, expected):
    user_norm = normalize_text(user_answer)
    score = 0
    feedback = []

    numbers = re.findall(r'\\d+', user_norm)
    numbers = [int(n) for n in numbers]

    expected_root = expected.get('root_value', None)
    expected_leaves = expected.get('visited_leaves', None)

    if expected_root is not None and expected_root in numbers:
        score += 50
        feedback.append(f"✓ Correct root value: {expected_root}")
    else:
        feedback.append(f"✗ Incorrect root value (expected: {expected_root})")

    if expected_leaves is not None and expected_leaves in numbers:
        score += 50
        feedback.append(f"✓ Correct visited leaves: {expected_leaves}")
    else:
        feedback.append(f"✗ Incorrect visited leaves (expected: {expected_leaves})")

    return min(score, 100), feedback

def evaluate_answer(question_type, user_answer, expected_answer):
    evaluators = {
        'search': evaluate_search,
        'game_theory': evaluate_game_theory,
        'csp': evaluate_csp,
        'minimax': evaluate_minimax
    }
    if question_type not in evaluators:
        return 0, ["Unsupported question type"]
    return evaluators[question_type](user_answer, expected_answer)
