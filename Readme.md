INSTALLATION GUIDE

This project requires Python 3.9 or higher. Follow these instructions to set up the environment and dependencies:

  1. Create a Virtual Environment (Recommended)

    It is best practice to isolate project dependencies. Open your terminal or command prompt and navigate to the project folder.

  On Windows:
  
    # Navigate to project folder first
    cd path\to\your\project
  
    # Create venv
    python -m venv venv
  
    # Activate venv
    venv\Scripts\activate

  On macOS / Linux:

    # Navigate to project folder first
    cd path/to/your/project

    # Create venv
    python3 -m venv venv

    # Activate venv
    source venv/bin/activate


  (You will see (venv) appear at the beginning of your terminal line if successful)

  2. Install Dependencies

    Once the virtual environment is activated, install all required packages:

    pip install -r requirements.txt


  3. Additional Requirement: C++ Compiler

    To run dynamic simulations (like Knight's Tour or Graph Coloring), you must have a C++ compiler installed on your system.

    Windows: Install MinGW-w64 and ensure g++ is added to your system PATH.

    Linux (Ubuntu/Debian):

      sudo apt-get update
      sudo apt-get install build-essential


    macOS: Install Xcode Command Line Tools:

      xcode-select --install

RUNNING THE APPLICATION:

  To start the application, you must be in the project's root directory (where app.py is located) and have your virtual environment activated.

  Open your terminal.

  Navigate to the project folder:

    cd path/to/your/project

  Activate the virtual environment (if not already active, see Step 1).

  Run the Streamlit command:

    streamlit run app.py

A browser window should automatically open with the application running at http://localhost:8501
