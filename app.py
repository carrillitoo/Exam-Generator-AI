import streamlit as st
import json
import pandas as pd
from pathlib import Path
from datetime import datetime

from evaluation import evaluate_answer
from pdf_utils import generate_question_pdf, generate_evaluation_pdf, extract_text_from_pdf

# Constants
QUESTIONS_JSON = "data/questions.json"
SUBMISSIONS_XLSX = "data/submissions.xlsx"

# Streamlit page config
st.set_page_config(page_title="AI Exam Generator", page_icon="🎓", layout="wide")

# Helpers: submissions Excel
def ensure_submissions_file(path=SUBMISSIONS_XLSX):
    p = Path(path)
    if not p.exists():
        df = pd.DataFrame(columns=[
            "submission_id", "timestamp_utc", "test_id", "question_id",
            "answer_text", "score", "feedback"
        ])
        df.to_excel(path, sheet_name="submissions", index=False)

def append_submission(test_id, question, user_answer, score, feedback, path=SUBMISSIONS_XLSX):
    ensure_submissions_file(path)
    df = pd.read_excel(path, sheet_name="submissions")
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
    with pd.ExcelWriter(path, engine="openpyxl", mode="w") as writer:
        df.to_excel(writer, sheet_name="submissions", index=False)

# Load questions from JSON
@st.cache_data
def load_questions_from_json(path=QUESTIONS_JSON):
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)

questions_db = load_questions_from_json()

# Session state init
if 'selected_questions' not in st.session_state:
    st.session_state.selected_questions = []
if 'current_question_idx' not in st.session_state:
    st.session_state.current_question_idx = 0
if 'answers' not in st.session_state:
    st.session_state.answers = {}
if 'evaluations' not in st.session_state:
    st.session_state.evaluations = {}
if 'test_id' not in st.session_state:
    st.session_state.test_id = "test_local"

# Sidebar
st.sidebar.title("AI Exam Generator")
page = st.sidebar.radio("Navigation", ["Generate Questions", "Answer Questions", "View Results"])
if st.sidebar.button("Reload questions"):
    st.cache_data.clear()
    questions_db = load_questions_from_json()
    st.success("Questions reloaded from JSON.")

# Page: Generate Questions
if page == "Generate Questions":
    st.header("📝 Generate Exam Questions")

    col1, col2 = st.columns(2)

    with col1:
        topics = sorted(list(set(q['topic'] for q in questions_db)))
        selected_topics = st.multiselect("Select Topics", topics, default=topics)

        difficulties = ['easy', 'medium', 'hard']
        selected_difficulties = st.multiselect("Select Difficulty", difficulties, default=difficulties)

        num_questions = st.slider("Number of Questions", 1, max(1, len(questions_db)), min(4, len(questions_db)))
        st.text_input("Test ID (for saving submissions)", value=st.session_state.test_id, key="test_id_input")
        st.session_state.test_id = st.session_state.test_id_input

    with col2:
        st.info("Available Types:\n- Search Strategies\n- Game Theory (Nash in normal form)\n- CSP (Backtracking + optimization)\n- Minimax with Alpha-Beta")

    if st.button("🎲 Generate Test", type="primary"):
        filtered = [
            q for q in questions_db
            if q['topic'] in selected_topics and q['difficulty'] in selected_difficulties
        ]
        if not filtered:
            st.error("No questions match your filters.")
        else:
            import random
            take = min(num_questions, len(filtered))
            st.session_state.selected_questions = random.sample(filtered, take)
            st.session_state.current_question_idx = 0
            st.session_state.answers = {}
            st.session_state.evaluations = {}
            st.success(f"Generated {len(st.session_state.selected_questions)} questions for test '{st.session_state.test_id}'.")

    if st.session_state.selected_questions:
        st.markdown("### 📋 Generated Questions")
        for i, q in enumerate(st.session_state.selected_questions, 1):
            with st.expander(f"Question {i}: {q['topic']} ({q['difficulty']})"):
                st.markdown(f"**Type:** {q['type']}")
                st.markdown(f"**Question:**\n\n{q['question']}")

        if st.button("📄 Export Questions to PDF"):
            pdf_file = generate_question_pdf(st.session_state.selected_questions)
            with open(pdf_file, 'rb') as f:
                st.download_button("⬇️ Download PDF", f, file_name="exam_questions.pdf", mime="application/pdf")

# Page: Answer Questions
elif page == "Answer Questions":
    st.header("✍️ Answer Questions")

    if not st.session_state.selected_questions:
        st.warning("Please generate questions first.")
    else:
        total = len(st.session_state.selected_questions)
        idx = st.session_state.current_question_idx

        if idx < total:
            q = st.session_state.selected_questions[idx]
            st.subheader(f"Question {idx + 1} of {total}")
            st.info(f"Topic: {q['topic']} | Type: {q['type']} | Difficulty: {q['difficulty']}")
            st.markdown(f"**Question:**\n\n{q['question']}")
            st.markdown("---")

            answer_method = st.radio("How to answer?", ["Type Answer", "Upload PDF"])
            user_answer = ""

            if answer_method == "Type Answer":
                user_answer = st.text_area("Your Answer", height=220, key=f"answer_{idx}")
            else:
                uploaded_file = st.file_uploader("Upload PDF", type=['pdf'], key=f"upload_{idx}")
                if uploaded_file:
                    user_answer = extract_text_from_pdf(uploaded_file)
                    if user_answer.startswith("Error extracting"):
                        st.error(user_answer)
                    else:
                        st.success("PDF text extracted.")
                        with st.expander("Extracted text"):
                            st.text(user_answer)

            c1, c2, c3 = st.columns(3)
            with c1:
                if idx > 0 and st.button("⬅️ Previous"):
                    st.session_state.current_question_idx -= 1
                    st.rerun()
            with c2:
                if st.button("💾 Save & Evaluate", type="primary"):
                    if user_answer and user_answer.strip():
                        st.session_state.answers[q['id']] = user_answer
                        score, feedback = evaluate_answer(q['type'], user_answer, q['expected_answer'])
                        st.session_state.evaluations[q['id']] = {
                            'score': score,
                            'feedback': feedback,
                            'question': q,
                            'user_answer': user_answer
                        }
                        # Persist submission to Excel
                        append_submission(st.session_state.test_id or "test_local", q, user_answer, score, feedback)
                        st.success(f"Answer saved and evaluated. Score: {score}%")
                    else:
                        st.error("Please provide an answer before saving.")
            with c3:
                if idx < total - 1 and st.button("Next ➡️"):
                    st.session_state.current_question_idx += 1
                    st.rerun()
        else:
            st.success("All questions answered. See results page.")

# Page: View Results
elif page == "View Results":
    st.header("📊 Evaluation Results")

    if not st.session_state.evaluations:
        st.warning("No evaluations yet.")
    else:
        total_score = sum(e['score'] for e in st.session_state.evaluations.values())
        avg_score = total_score / len(st.session_state.evaluations)

        c1, c2, c3 = st.columns(3)
        c1.metric("Questions Answered", len(st.session_state.evaluations))
        c2.metric("Average Score", f"{avg_score:.1f}%")
        c3.metric("Total Points", f"{total_score}/{len(st.session_state.evaluations) * 100}")

        st.markdown("---")

        for i, (qid, data) in enumerate(st.session_state.evaluations.items(), 1):
            q = data['question']
            score = data['score']
            feedback = data['feedback']
            user_answer = data['user_answer']

            with st.expander(f"Question {i}: {q['topic']} - Score: {score}%"):
                st.markdown(f"**Question:**\n\n{q['question']}")
                st.markdown("**Your Answer:**")
                st.markdown(user_answer)

                if score >= 70:
                    st.success(f"Score: {score}%")
                elif score >= 50:
                    st.warning(f"Score: {score}%")
                else:
                    st.error(f"Score: {score}%")

                st.markdown("**Feedback:**")
                for item in feedback:
                    st.markdown(f"- {item}")

                st.markdown("**Correct Answer:**")
                expected = q['expected_answer']
                if isinstance(expected, dict):
                    for key, value in expected.items():
                        st.markdown(f"- {key.capitalize()}: {value}")
                else:
                    st.markdown(str(expected))

                if st.button(f"📄 Export Evaluation {i} to PDF", key=f"export_{i}"):
                    pdf_file = generate_evaluation_pdf(q, user_answer, score, feedback, expected, f"evaluation_{i}.pdf")
                    with open(pdf_file, 'rb') as f:
                        st.download_button(f"⬇️ Download Evaluation {i}", f, file_name=f"evaluation_{i}.pdf", mime="application/pdf", key=f"download_{i}")
