import streamlit as st
import json
import pandas as pd
from pathlib import Path
from datetime import datetime
import subprocess
import os
import re
import platform
import random 

# Importamos solo la evaluación y utilidades PDF
from evaluation import evaluate_answer
from pdf_utils import generate_question_pdf, generate_evaluation_pdf, extract_text_from_pdf

# --- Configuración y Rutas ---
BASE_DIR = Path(__file__).parent
DATA_DIR = BASE_DIR / "data"
CPP_DIR = DATA_DIR / "cpp"  
QUESTIONS_FILE = DATA_DIR / "questions.json"
SUBMISSIONS_FILE = DATA_DIR / "submissions.xlsx"

st.set_page_config(page_title="AI Exam Generator", page_icon="🎓", layout="wide")

# --- Helper: Ejecución Dinámica C++ ---
def run_cpp_simulation(file_name):
    """
    Compila y ejecuta un archivo C++, extrayendo el algoritmo ganador.
    Soporta salidas en inglés ("Algorithm:") y español ("Algoritmo:").
    """
    source_path = CPP_DIR / file_name
    if not source_path.exists():
        return None, f"Error: File '{file_name}' not found at '{source_path}'."

    exe_name = source_path.with_suffix('.exe' if platform.system() == "Windows" else '.out')
    
    # Compilación (Optimizada -O3)
    compile_cmd = ["g++", "-O3", str(source_path), "-o", str(exe_name)]
    try:
        subprocess.run(compile_cmd, check=True, capture_output=True)
    except subprocess.CalledProcessError as e:
        return None, f"Compilation Error: {e.stderr.decode()}"
    except FileNotFoundError:
        return None, "Error: 'g++' compiler not found. Please install MinGW (Windows) or build-essential (Linux)."

    # Ejecución
    try:
        cmd = [str(exe_name)]
        # Timeout de 5 segundos para seguridad
        result = subprocess.run(cmd, check=True, capture_output=True, text=True, timeout=5)
        output = result.stdout
        
        # --- PARSER FLEXIBLE ---
        match = re.search(r">\s*(?:Algoritmo|Algorithm):\s*(.*)", output, re.IGNORECASE)
        
        if match:
            best_algo = match.group(1).strip()
            return best_algo, None
        else:
            lines = output.strip().split('\n')
            last_lines = "\n".join(lines[-5:]) if len(lines) > 5 else output
            return None, f"Error: Output format not recognized.\nLast output:\n{last_lines}"
            
    except subprocess.TimeoutExpired:
        return None, "Error: Execution timed out (infinite loop or too slow)."
    except Exception as e:
        return None, f"Execution Error: {str(e)}"

# --- Helpers: Submissions ---
def ensure_submissions_file():
    if not DATA_DIR.exists():
        DATA_DIR.mkdir(parents=True, exist_ok=True)
    if not SUBMISSIONS_FILE.exists():
        df = pd.DataFrame(columns=["submission_id", "timestamp_utc", "test_id", "question_id", "answer_text", "score", "feedback"])
        df.to_excel(SUBMISSIONS_FILE, sheet_name="submissions", index=False)

def append_submission(test_id, question, user_answer, score, feedback):
    ensure_submissions_file()
    try:
        df = pd.read_excel(SUBMISSIONS_FILE, sheet_name="submissions")
        new_row = {
            "submission_id": f"sub_{int(datetime.utcnow().timestamp())}",
            "timestamp_utc": datetime.utcnow().isoformat(timespec="seconds"),
            "test_id": test_id,
            "question_id": question["id"],
            "answer_text": user_answer,
            "score": score,
            "feedback": "; ".join(feedback)
        }
        df = pd.concat([df, pd.DataFrame([new_row])], ignore_index=True)
        with pd.ExcelWriter(SUBMISSIONS_FILE, engine="openpyxl", mode="w") as writer:
            df.to_excel(writer, sheet_name="submissions", index=False)
    except Exception as e:
        st.error(f"Failed to save submission: {e}")

@st.cache_data
def load_questions_from_json():
    if not QUESTIONS_FILE.exists(): return None
    try:
        with open(QUESTIONS_FILE, "r", encoding="utf-8") as f:
            return json.load(f)
    except json.JSONDecodeError: return []

questions_db = load_questions_from_json()

# Inicializar Estado de Sesión
if 'selected_questions' not in st.session_state: st.session_state.selected_questions = []
if 'current_question_idx' not in st.session_state: st.session_state.current_question_idx = 0
if 'answers' not in st.session_state: st.session_state.answers = {}
if 'evaluations' not in st.session_state: st.session_state.evaluations = {}
if 'test_id' not in st.session_state: st.session_state.test_id = "test_local"

# --- Interfaz de Usuario ---
st.sidebar.title("AI Exam Generator")
page = st.sidebar.radio("Navigation", ["Generate Questions", "Answer Questions", "View Results"])

if st.sidebar.button("Reload Questions"):
    st.cache_data.clear()
    questions_db = load_questions_from_json()
    st.success("Questions reloaded.")

# 1. GENERAR TEST
if page == "Generate Questions":
    st.header("📝 Configure and Generate Test")
    col1, col2 = st.columns(2)
    
    with col1:
        if not questions_db:
            st.error("No questions loaded.")
        else:
            topics = sorted(list(set(q.get('topic', 'General') for q in questions_db)))
            selected_topics = st.multiselect("Select Topics", topics, default=topics)
            num_questions = st.slider("Number of Questions", 1, 20, 5) 
            st.session_state.test_id = st.text_input("Test ID", value=st.session_state.test_id)

    with col2:
        st.info("System Ready. Using **Fuzzy Logic Evaluation**.")
        st.caption("Answers are checked flexibly (e.g., partial matches).")

    if st.button("🎲 Generate Test", type="primary"):
        # Filtrar preguntas
        filtered = [q for q in questions_db if q.get('topic') in selected_topics]
        
        if not filtered:
            st.error("No questions match filters.")
        else:
            # Selección aleatoria simple de la base de datos
            count = min(num_questions, len(filtered))
            # random.sample asegura que no haya duplicados si el pool es suficiente
            selected = random.sample(filtered, count)
            
            final_selection = []
            progress_bar = st.progress(0)
            status_text = st.empty()
            
            for i, q in enumerate(selected):
                q_copy = q.copy()
                
                # Ejecutar simulación si es necesario (C++)
                if q_copy.get('type') == 'dynamic_algo':
                    status_text.text(f"Simulating: {q_copy.get('code_file')}...")
                    best_algo, error = run_cpp_simulation(q_copy['code_file'])
                    
                    if error:
                        q_copy['expected_answer'] = {'strategy': "Error in simulation"}
                        # Opcional: st.warning(f"Sim error: {error}")
                    else:
                        q_copy['expected_answer'] = {'strategy': best_algo}
                
                final_selection.append(q_copy)
                progress_bar.progress((i + 1) / count)
            
            status_text.empty()
            progress_bar.empty()
            
            st.session_state.selected_questions = final_selection
            st.session_state.current_question_idx = 0
            st.session_state.answers = {}
            st.session_state.evaluations = {}
            st.success(f"Generated {len(final_selection)} questions for Test ID: {st.session_state.test_id}")

    # --- SECCIÓN DE PREVISUALIZACIÓN ---
    if st.session_state.selected_questions:
        st.markdown("### 📋 Test Preview")
        for i, q in enumerate(st.session_state.selected_questions, 1):
            with st.expander(f"📄 Q{i}: {q.get('topic')} ({q.get('type')})"):
                st.markdown(q['question'])
                # Mostrar respuesta esperada (calculada o estática)
                if q.get('type') == 'dynamic_algo':
                    strategy = q.get('expected_answer', {}).get('strategy', 'Pending/Error')
                    st.caption(f"⚡ *System-calculated answer:* {strategy}")
                elif 'expected_answer' in q:
                    # Manejar si es string o dict
                    ans = q['expected_answer']
                    if isinstance(ans, dict): ans = ans.get('text', str(ans))
                    st.caption(f"🔑 *Expected:* {ans}")

# 2. RESPONDER PREGUNTAS
elif page == "Answer Questions":
    st.header("✍️ Take Exam")
    
    if not st.session_state.selected_questions:
        st.warning("Generate a test first.")
    else:
        idx = st.session_state.current_question_idx
        total = len(st.session_state.selected_questions)
        q = st.session_state.selected_questions[idx]
        
        st.subheader(f"Question {idx + 1} of {total}")
        st.info(f"Topic: {q.get('topic')} | Difficulty: {q.get('difficulty', 'Normal')}")
        st.markdown(q['question'])
        
        user_answer = st.text_area("Your Answer", height=150, key=f"ans_{idx}")
        
        c1, c2, c3 = st.columns(3)
        if idx > 0 and c1.button("⬅️ Previous"):
            st.session_state.current_question_idx -= 1
            st.rerun()
            
        if c2.button("💾 Save & Evaluate", type="primary"):
            if user_answer.strip():
                # Evaluación flexible usando el objeto completo 'q'
                score, feedback = evaluate_answer(q, user_answer)
                
                st.session_state.answers[q['id']] = user_answer
                st.session_state.evaluations[q['id']] = {
                    'score': score, 
                    'feedback': feedback, 
                    'question': q, 
                    'user_answer': user_answer
                }
                append_submission(st.session_state.test_id, q, user_answer, score, feedback)
                
                color = "green" if score >= 80 else "orange" if score >= 50 else "red"
                st.markdown(f":{color}[**Score: {score}%**]")
                for f in feedback: st.write(f"- {f}")
            else:
                st.error("Empty answer.")
                
        if idx < total - 1 and c3.button("Next ➡️"):
            st.session_state.current_question_idx += 1
            st.rerun()

# 3. VER RESULTADOS
elif page == "View Results":
    st.header("📊 Results")
    if not st.session_state.evaluations:
        st.warning("No answers yet.")
    else:
        total_score = sum(e['score'] for e in st.session_state.evaluations.values())
        avg = total_score / len(st.session_state.evaluations)
        st.metric("Average Score", f"{avg:.1f}%")
        
        for qid, data in st.session_state.evaluations.items():
            q = data['question']
            with st.expander(f"Q: {q.get('topic')} - {data['score']}%"):
                st.write(f"**Question:** {q['question']}")
                st.write(f"**You:** {data['user_answer']}")
                st.markdown("---")
                st.write("**AI Feedback:**")
                for f in data['feedback']: st.write(f"- {f}")
                
                # Mostrar respuesta correcta
                if 'valid_responses' in q:
                    best_response = q['valid_responses'][0]['text']
                    st.info(f"Ideal Answer: {best_response}")
                elif 'expected_answer' in q:
                    ans = q['expected_answer']
                    if isinstance(ans, dict): ans = ans.get('strategy', str(ans))
                    st.info(f"Expected: {ans}")
