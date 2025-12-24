from thefuzz import fuzz
import re

# --- ALIAS PARA PREGUNTAS DINÁMICAS ---
DYNAMIC_ALGO_ALIASES = {
    "hill climbing": ["hill climbing", "hc", "escalada", "ascenso de colinas", "local search", "warnsdorff"],
    "backtracking": ["backtracking", "bt", "vuelta atras", "fuerza bruta", "dfs"],
    "dfs": ["dfs", "depth first", "profundidad"],
    "bfs": ["bfs", "breadth first", "anchura"],
    "a*": ["a*", "a star", "a-star", "a start", "heuristica"],
    "(d, r)": ["(d, r)", "d,r", "down right", "abajo derecha"],
    "(u, l)": ["(u, l)", "u,l", "up left", "arriba izquierda"],
    "(u, r)": ["(u, r)", "u,r", "up right", "arriba derecha"],
    "(d, l)": ["(d, l)", "d,l", "down left", "abajo izquierda"],
    "(coop, coop)": ["(coop, coop)", "coop", "both silent"],
    "(betray, betray)": ["(betray, betray)", "betray", "confess", "defect"],
    "no": ["no", "none", "ninguno", "no existe"],
    "yes": ["yes", "si", "sí", "existe"]
}

def normalize_csp_string(text):
    """Parsea 'A=1, B=2' a diccionario."""
    text = text.lower()
    matches = re.findall(r'([a-zA-Z0-9_]+)\s*[=:]\s*([a-zA-Z0-9_]+)', text)
    return {k.strip(): v.strip() for k, v in matches}

def extract_vector_from_text(text):
    """
    Busca y extrae un vector numérico de un texto con ruido.
    Ej: "La respuesta es [1, 2, 3]" -> ['1', '2', '3']
    Ej: "1, 2, 3" -> ['1', '2', '3']
    """
    # 1. Intentar encontrar patrón estricto entre corchetes
    bracket_match = re.search(r'\[([\d\s,]+)\]', text)
    if bracket_match:
        content = bracket_match.group(1)
        # Dividir por coma o espacio
        return [x.strip() for x in re.split(r'[,\s]+', content) if x.strip()]
    
    # 2. Si no hay corchetes, intentar buscar secuencia de números separados
    # Ej: "0 3 1 2"
    # Solo si el texto parece ser mayormente números
    digits = re.findall(r'\b\d+\b', text)
    if len(digits) >= 3: # Asumimos vector si hay al menos 3 números
        return digits
        
    return []

def evaluate_vector_structure(user_input, correct_output):
    """
    Evaluación específica para vectores (N-Queens).
    Extrae el vector del ruido y compara elemento a elemento.
    """
    # Extraer vectores limpios
    user_vec = extract_vector_from_text(user_input)
    correct_vec = extract_vector_from_text(correct_output)

    if not correct_vec:
        # Fallback si no hay vector en la respuesta correcta
        return evaluate_fuzzy(user_input, [{"text": correct_output, "score": 100}])

    if not user_vec:
        return 0, ["No se detectó un vector válido en tu respuesta. Usa el formato [1, 2, 3]."]

    # Comparación de longitud
    if len(user_vec) != len(correct_vec):
        return 0, [f"Longitud incorrecta. Esperados {len(correct_vec)} elementos, encontrados {len(user_vec)}."]

    matches = 0
    feedback = []
    
    for i, (u_val, c_val) in enumerate(zip(user_vec, correct_vec)):
        if u_val == c_val:
            matches += 1
            feedback.append(f"✓ Pos {i}: {u_val} (Correcto)")
        else:
            feedback.append(f"✗ Pos {i}: Esperado {c_val}, tu pusiste {u_val}")

    score = int((matches / len(correct_vec)) * 100)
    
    if score == 100:
        feedback.insert(0, "¡Excelente! Vector exacto.")
    elif score > 0:
        feedback.insert(0, f"Vector parcialmente correcto ({matches}/{len(correct_vec)} aciertos).")
    else:
        feedback.insert(0, "Ninguna posición coincide.")

    return score, feedback

def evaluate_csp_structure(user_input, correct_output):
    """Evaluación para mapas/variables (SA=Red, etc)."""
    user_assignments = normalize_csp_string(user_input)
    correct_assignments = normalize_csp_string(correct_output)

    if not correct_assignments:
        return evaluate_fuzzy(user_input, [{"text": correct_output, "score": 100}])

    if not user_assignments:
        return 0, ["Formato no reconocido. Usa 'Variable=Valor' (ej: SA=Red)."]

    total_vars = len(correct_assignments)
    matches = 0
    feedback = []

    for var, correct_val in correct_assignments.items():
        found_var = None
        for u_var in user_assignments:
            if u_var.lower() == var.lower():
                found_var = u_var
                break
        
        if found_var:
            user_val = user_assignments[found_var]
            if user_val.lower() == correct_val.lower():
                matches += 1
                feedback.append(f"✓ {var.upper()}={correct_val} (Correcto)")
            else:
                feedback.append(f"✗ {var.upper()}: Esperado '{correct_val}', tú pusiste '{user_val}'")
        else:
            feedback.append(f"✗ Falta la variable {var.upper()}")

    score = int((matches / total_vars) * 100)
    if score == 100: feedback.insert(0, "¡Correcto! Asignación perfecta.")
    elif score > 0: feedback.insert(0, f"Solución parcial ({matches}/{total_vars}).")
    else: feedback.insert(0, "Incorrecto.")
    
    return score, feedback

def evaluate_fuzzy(user_input, valid_options_list):
    best_score = 0
    matched_text = ""
    clean_input = str(user_input).lower().strip()

    for option in valid_options_list:
        target_text = option["text"]
        max_points = option.get("score", 100)
        similarity = fuzz.token_set_ratio(clean_input, target_text.lower())
        
        if similarity >= 55:
            calculated_score = (similarity / 100.0) * max_points
            if calculated_score > best_score:
                best_score = calculated_score
                matched_text = target_text

    final_score = int(min(100, max(0, round(best_score))))
    feedback = []
    if final_score == 100: feedback.append("✓ Respuesta exacta.")
    elif final_score >= 50: feedback.append(f"⚠ Correcto con imprecisiones (Ref: '{matched_text}')")
    else: feedback.append("✗ Incorrecto.")
    return final_score, feedback

def evaluate_answer(question_data, user_answer):
    q_type = question_data.get('type', '')
    
    # Lógica Dinámica
    if q_type == 'dynamic_algo':
        winning_algo_raw = question_data.get('expected_answer', {}).get('strategy', '')
        
        if not winning_algo_raw or "error" in winning_algo_raw.lower():
            return 0, ["Error en simulación."]

        # 1. ¿Es un Vector? (Empieza por corchete '[')
        if winning_algo_raw.strip().startswith('['):
            return evaluate_vector_structure(user_answer, winning_algo_raw)

        # 2. ¿Es CSP Map/Logic? (Contiene '=')
        if '=' in winning_algo_raw:
            return evaluate_csp_structure(user_answer, winning_algo_raw)
        
        # 3. Algoritmo estándar (Fuzzy)
        valid_responses = [{"text": winning_algo_raw, "score": 100}]
        detected_key = None
        for key in DYNAMIC_ALGO_ALIASES:
            if key in winning_algo_raw.lower():
                detected_key = key
                break
        if detected_key:
            for alias in DYNAMIC_ALGO_ALIASES[detected_key]:
                valid_responses.append({"text": alias, "score": 100})
                
        return evaluate_fuzzy(user_answer, valid_responses)

    # Lógica Estática
    if 'valid_responses' in question_data:
        first_valid = question_data['valid_responses'][0]['text']
        if first_valid.strip().startswith('['):
            return evaluate_vector_structure(user_answer, first_valid)
        if '=' in first_valid:
            return evaluate_csp_structure(user_answer, first_valid)
             
        return evaluate_fuzzy(user_answer, question_data['valid_responses'])
    
    return 0, ["No evaluable."]
