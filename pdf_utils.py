from fpdf import FPDF
import PyPDF2

class PDF(FPDF):
    def header(self):
        self.set_font('Arial', 'B', 12)
        self.cell(0, 10, 'AI Exam Generator', 0, 1, 'C')
        self.ln(3)

    def footer(self):
        self.set_y(-15)
        self.set_font('Arial', 'I', 8)
        self.cell(0, 10, f'Page {self.page_no()}', 0, 0, 'C')

def generate_question_pdf(questions, filename='questions.pdf'):
    pdf = PDF()
    pdf.add_page()
    pdf.set_font('Arial', '', 11)

    for i, q in enumerate(questions, 1):
        pdf.set_font('Arial', 'B', 12)
        pdf.cell(0, 8, f"Question {i} - {q['topic']}", 0, 1)
        pdf.set_font('Arial', 'I', 9)
        pdf.cell(0, 5, f"Type: {q['type']} | Difficulty: {q['difficulty']}", 0, 1)
        pdf.ln(2)

        pdf.set_font('Arial', '', 10)
        pdf.multi_cell(0, 5, q['question'])
        pdf.ln(6)

    pdf.output(filename)
    return filename

def generate_evaluation_pdf(question, user_answer, score, feedback, correct_answer, filename='evaluation.pdf'):
    pdf = PDF()
    pdf.add_page()

    pdf.set_font('Arial', 'B', 12)
    pdf.cell(0, 8, 'Question:', 0, 1)
    pdf.set_font('Arial', '', 10)
    pdf.multi_cell(0, 5, question['question'])
    pdf.ln(3)

    pdf.set_font('Arial', 'B', 12)
    pdf.cell(0, 8, 'Your Answer:', 0, 1)
    pdf.set_font('Arial', '', 10)
    pdf.multi_cell(0, 5, user_answer)
    pdf.ln(3)

    pdf.set_font('Arial', 'B', 14)
    color = (0, 150, 0) if score >= 70 else (255, 140, 0) if score >= 50 else (200, 0, 0)
    pdf.set_text_color(*color)
    pdf.cell(0, 10, f'Score: {score}%', 0, 1)
    pdf.set_text_color(0, 0, 0)
    pdf.ln(2)

    pdf.set_font('Arial', 'B', 12)
    pdf.cell(0, 8, 'Feedback:', 0, 1)
    pdf.set_font('Arial', '', 10)
    for item in feedback:
        pdf.multi_cell(0, 5, f'- {item}')
    pdf.ln(3)

    pdf.set_font('Arial', 'B', 12)
    pdf.cell(0, 8, 'Correct Answer:', 0, 1)
    pdf.set_font('Arial', '', 10)
    if isinstance(correct_answer, dict):
        for key, value in correct_answer.items():
            pdf.set_font('Arial', 'B', 10)
            pdf.cell(0, 5, f'{key.capitalize()}:', 0, 1)
            pdf.set_font('Arial', '', 10)
            pdf.multi_cell(0, 5, f'{value}')
    else:
        pdf.multi_cell(0, 5, str(correct_answer))

    pdf.output(filename)
    return filename

def extract_text_from_pdf(pdf_file):
    try:
        reader = PyPDF2.PdfReader(pdf_file)
        text = ''
        for page in reader.pages:
            text += page.extract_text() or ''
        return text
    except Exception as e:
        return f"Error extracting text: {str(e)}"
