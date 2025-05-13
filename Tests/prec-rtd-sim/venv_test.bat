py -m venv .\venv --clear
call .\venv\Scripts\activate
pip install -r requirements.txt
py .\VisualRtdSim.py
deactivate