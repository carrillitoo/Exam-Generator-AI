import re
import json

def normalize_text(text):
    """Normaliza texto para comparación: minúsculas, sin espacios extra."""
    return re.sub(r'\s+', ' ', text.lower().strip())

def evaluate_search(user_answer, expected):
    """Evalúa respuesta de tipo 'search' (estrategia de búsqueda)."""
    user_norm = normalize_text(user_answer)
    expected_strategy = normalize_text(expected['strategy'])
    
    score = 0
    feedback = []
    
    # Verificar si menciona la estrategia correcta
    if expected_strategy in user_norm:
        score += 60
        feedback.append("✓ Estrategia correcta identificada")
    else:
        feedback.append("✗ Estrategia incorrecta o no identificada")
    
    # Verificar si incluye justificación (palabras clave)
    keywords = ['constraint', 'prune', 'backtrack', 'systematic', 'valid']
    found_keywords = sum(1 for kw in keywords if kw in user_norm)
    
    if found_keywords >= 2:
        score += 40
        feedback.append("✓ Justificación adecuada")
    elif found_keywords == 1:
        score += 20
        feedback.append("⚠ Justificación parcial")
    else:
        feedback.append("✗ Falta justificación")
    
    return min(score, 100), feedback

def evaluate_game_theory(user_answer, expected):
    """Evalúa respuesta de tipo 'game_theory' (equilibrio de Nash)."""
    user_norm = normalize_text(user_answer)
    
    score = 0
    feedback = []
    
    # Verificar existencia
    exists = expected['exists']
    if (exists and any(word in user_norm for word in ['yes', 'sí', 'si', 'existe', 'exists'])) or \
       (not exists and any(word in user_norm for word in ['no', 'not exist', 'no existe'])):
        score += 40
        feedback.append("✓ Existencia correcta")
    else:
        feedback.append("✗ Existencia incorrecta")
    
    # Verificar equilibrio identificado
    if exists:
        equilibrium_norm = normalize_text(expected['equilibrium'])
        # Extraer coordenadas (D,R) o D R o similar
        if equilibrium_norm.replace(',', '').replace('(', '').replace(')', '') in user_norm.replace(',', '').replace('(', '').replace(')', ''):
            score += 30
            feedback.append("✓ Equilibrio identificado correctamente")
        else:
            feedback.append("✗ Equilibrio incorrecto o no identificado")
    
    # Verificar justificación
    justification_keywords = ['deviate', 'improve', 'incentive', 'mejor', 'cambiar', 'unilateral']
    found = sum(1 for kw in justification_keywords if kw in user_norm)
    
    if found >= 2:
        score += 30
        feedback.append("✓ Justificación completa")
    elif found == 1:
        score += 15
        feedback.append("⚠ Justificación parcial")
    else:
        feedback.append("✗ Falta justificación")
    
    return min(score, 100), feedback

def evaluate_csp(user_answer, expected):
    """Evalúa respuesta de tipo 'csp' (asignación final)."""
    user_norm = normalize_text(user_answer)
    expected_assignment = normalize_text(expected['assignment'])
    
    score = 0
    feedback = []
    
    # Extraer asignaciones del formato "X=1, Y=2, Z=3"
    expected_pairs = re.findall(r'([a-z])=(\d+)', expected_assignment)
    user_pairs = re.findall(r'([a-z])=(\d+)', user_norm)
    
    expected_dict = {var: val for var, val in expected_pairs}
    user_dict = {var: val for var, val in user_pairs}
    
    correct = 0
    total = len(expected_dict)
    
    for var, val in expected_dict.items():
        if user_dict.get(var) == val:
            correct += 1
            feedback.append(f"✓ {var.upper()}={val} correcto")
        else:
            feedback.append(f"✗ {var.upper()} incorrecto (esperado: {val}, obtenido: {user_dict.get(var, 'N/A')})")
    
    score = int((correct / total) * 70) if total > 0 else 0
    
    # Bonus por explicación
    if any(word in user_norm for word in ['forward checking', 'domain', 'reduce', 'constraint']):
        score += 30
        feedback.append("✓ Explicación del proceso incluida")
    else:
        feedback.append("⚠ Falta explicación del proceso")
    
    return min(score, 100), feedback

def evaluate_minimax(user_answer, expected):
    """Evalúa respuesta de tipo 'minimax' (valor raíz y hojas visitadas)."""
    user_norm = normalize_text(user_answer)
    
    score = 0
    feedback = []
    
    # Extraer números de la respuesta del usuario
    numbers = re.findall(r'\d+', user_answer)
    numbers = [int(n) for n in numbers]
    
    expected_root = expected['root_value']
    expected_leaves = expected['visited_leaves']
    
    # Verificar valor raíz
    if expected_root in numbers:
        score += 50
        feedback.append(f"✓ Valor raíz correcto: {expected_root}")
    else:
        feedback.append(f"✗ Valor raíz incorrecto (esperado: {expected_root})")
    
    # Verificar hojas visitadas
    if expected_leaves in numbers:
        score += 50
        feedback.append(f"✓ Hojas visitadas correcto: {expected_leaves}")
    else:
        feedback.append(f"✗ Hojas visitadas incorrecto (esperado: {expected_leaves})")
    
    return min(score, 100), feedback

def evaluate_answer(question_type, user_answer, expected_answer):
    """Función principal de evaluación que delega según el tipo."""
    evaluators = {
        'search': evaluate_search,
        'game_theory': evaluate_game_theory,
        'csp': evaluate_csp,
        'minimax': evaluate_minimax
    }
    
    if question_type not in evaluators:
        return 0, ["Tipo de pregunta no soportado"]
    
    return evaluators[question_type](user_answer, expected_answer)
