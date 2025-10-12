import streamlit as st
import json
from evaluacion import evaluate_answer
from pdf_utils import generate_question_pdf, generate_evaluation_pdf, extract_text_from_pdf

# Configuración de la página
st.set_page_config(page_title="AI Exam Generator", page_icon="🎓", layout="wide")

# Cargar preguntas
@st.cache_data
def load_questions():
    with open('data/questions.json', 'r', encoding='utf-8') as f:
        return json.load(f)

questions_db = load_questions()

# Inicializar estado de sesión
if 'selected_questions' not in st.session_state:
    st.session_state.selected_questions = []
if 'current_question_idx' not in st.session_state:
    st.session_state.current_question_idx = 0
if 'answers' not in st.session_state:
    st.session_state.answers = {}
if 'evaluations' not in st.session_state:
    st.session_state.evaluations = {}

# Título principal
st.title("🎓 AI Exam Generator")
st.markdown("---")

# Sidebar para navegación
page = st.sidebar.radio("Navigation", ["Generate Questions", "Answer Questions", "View Results"])

# ========== PÁGINA 1: GENERAR PREGUNTAS ==========
if page == "Generate Questions":
    st.header("📝 Generate Exam Questions")
    
    col1, col2 = st.columns(2)
    
    with col1:
        # Filtros
        topics = list(set([q['topic'] for q in questions_db]))
        selected_topics = st.multiselect("Select Topics", topics, default=topics)
        
        difficulties = ['easy', 'medium', 'hard']
        selected_difficulties = st.multiselect("Select Difficulty", difficulties, default=difficulties)
        
        num_questions = st.slider("Number of Questions", 1, len(questions_db), 4)
    
    with col2:
        st.info("**Available Question Types:**\n- Search Strategies\n- Game Theory (Nash Equilibrium)\n- Constraint Satisfaction Problems\n- Minimax with Alpha-Beta Pruning")
    
    if st.button("🎲 Generate Test", type="primary"):
        # Filtrar preguntas
        filtered = [q for q in questions_db 
                   if q['topic'] in selected_topics 
                   and q['difficulty'] in selected_difficulties]
        
        if len(filtered) < num_questions:
            st.warning(f"Only {len(filtered)} questions match your criteria.")
            st.session_state.selected_questions = filtered
        else:
            import random
            st.session_state.selected_questions = random.sample(filtered, num_questions)
        
        st.session_state.current_question_idx = 0
        st.session_state.answers = {}
        st.session_state.evaluations = {}
        st.success(f"✅ Generated {len(st.session_state.selected_questions)} questions!")
    
    # Mostrar preguntas generadas
    if st.session_state.selected_questions:
        st.markdown("---")
        st.subheader("📋 Generated Questions")
        
        for i, q in enumerate(st.session_state.selected_questions, 1):
            with st.expander(f"Question {i}: {q['topic']} ({q['difficulty']})"):
                st.markdown(f"**Type:** {q['type']}")
                st.markdown(f"**Question:**\n\n{q['question']}")
        
        # Botón para exportar a PDF
        if st.button("📄 Export Questions to PDF"):
            pdf_file = generate_question_pdf(st.session_state.selected_questions)
            with open(pdf_file, 'rb') as f:
                st.download_button("⬇️ Download PDF", f, file_name="exam_questions.pdf", mime="application/pdf")

# ========== PÁGINA 2: RESPONDER PREGUNTAS ==========
elif page == "Answer Questions":
    st.header("✍️ Answer Questions")
    
    if not st.session_state.selected_questions:
        st.warning("⚠️ Please generate questions first in the 'Generate Questions' page.")
    else:
        total = len(st.session_state.selected_questions)
        idx = st.session_state.current_question_idx
        
        if idx < total:
            q = st.session_state.selected_questions[idx]
            
            st.subheader(f"Question {idx + 1} of {total}")
            st.info(f"**Topic:** {q['topic']} | **Type:** {q['type']} | **Difficulty:** {q['difficulty']}")
            st.markdown(f"**Question:**\n\n{q['question']}")
            
            st.markdown("---")
            
            # Opciones de respuesta
            answer_method = st.radio("How would you like to answer?", ["Type Answer", "Upload PDF"])
            
            user_answer = ""
            
            if answer_method == "Type Answer":
                user_answer = st.text_area("Your Answer:", height=200, key=f"answer_{idx}")
            else:
                uploaded_file = st.file_uploader("Upload PDF with your answer", type=['pdf'], key=f"upload_{idx}")
                if uploaded_file:
                    user_answer = extract_text_from_pdf(uploaded_file)
                    st.success("✅ PDF text extracted successfully!")
                    with st.expander("View extracted text"):
                        st.text(user_answer)
            
            col1, col2, col3 = st.columns([1, 1, 1])
            
            with col1:
                if idx > 0:
                    if st.button("⬅️ Previous"):
                        st.session_state.current_question_idx -= 1
                        st.rerun()
            
            with col2:
                if st.button("💾 Save & Evaluate", type="primary"):
                    if user_answer.strip():
                        st.session_state.answers[q['id']] = user_answer
                        
                        # Evaluar
                        score, feedback = evaluate_answer(q['type'], user_answer, q['expected_answer'])
                        st.session_state.evaluations[q['id']] = {
                            'score': score,
                            'feedback': feedback,
                            'question': q,
                            'user_answer': user_answer
                        }
                        
                        st.success(f"✅ Answer saved and evaluated! Score: {score}%")
                    else:
                        st.error("Please provide an answer before saving.")
            
            with col3:
                if idx < total - 1:
                    if st.button("Next ➡️"):
                        st.session_state.current_question_idx += 1
                        st.rerun()
        else:
            st.success("🎉 You've answered all questions! Go to 'View Results' to see your evaluation.")

# ========== PÁGINA 3: VER RESULTADOS ==========
elif page == "View Results":
    st.header("📊 Evaluation Results")
    
    if not st.session_state.evaluations:
        st.warning("⚠️ No evaluations yet. Please answer questions first.")
    else:
        # Resumen general
        total_score = sum([e['score'] for e in st.session_state.evaluations.values()])
        avg_score = total_score / len(st.session_state.evaluations)
        
        col1, col2, col3 = st.columns(3)
        col1.metric("Questions Answered", len(st.session_state.evaluations))
        col2.metric("Average Score", f"{avg_score:.1f}%")
        col3.metric("Total Points", f"{total_score}/{len(st.session_state.evaluations) * 100}")
        
        st.markdown("---")
        
        # Resultados individuales
        for i, (q_id, eval_data) in enumerate(st.session_state.evaluations.items(), 1):
            q = eval_data['question']
            score = eval_data['score']
            feedback = eval_data['feedback']
            user_answer = eval_data['user_answer']
            
            with st.expander(f"Question {i}: {q['topic']} - Score: {score}%"):
                st.markdown(f"**Question:**\n\n{q['question']}")
                st.markdown("---")
                st.markdown(f"**Your Answer:**\n\n{user_answer}")
                st.markdown("---")
                
                # Puntuación con color
                if score >= 70:
                    st.success(f"**Score: {score}%** ✅")
                elif score >= 50:
                    st.warning(f"**Score: {score}%** ⚠️")
                else:
                    st.error(f"**Score: {score}%** ❌")
                
                st.markdown("**Feedback:**")
                for item in feedback:
                    st.markdown(f"- {item}")
                
                st.markdown("---")
                st.markdown("**Correct Answer:**")
                
                expected = q['expected_answer']
                if isinstance(expected, dict):
                    for key, value in expected.items():
                        st.markdown(f"**{key.capitalize()}:** {value}")
                else:
                    st.markdown(expected)
                
                # Botón para exportar evaluación individual a PDF
                if st.button(f"📄 Export Evaluation {i} to PDF", key=f"export_{i}"):
                    pdf_file = generate_evaluation_pdf(q, user_answer, score, feedback, expected, f"evaluation_{i}.pdf")
                    with open(pdf_file, 'rb') as f:
                        st.download_button(f"⬇️ Download Evaluation {i}", f, file_name=f"evaluation_{i}.pdf", mime="application/pdf", key=f"download_{i}")

st.sidebar.markdown("---")
st.sidebar.info("**AI Exam Generator v1.0**\n\nDeveloped for Artificial Intelligence course project.")
