; Ask the user to install the appropriate version of Python.
Section "-Prerequisites"
  SetOutPath Prereqs
  MessageBox MB_YESNO "For OpenNERO to run correctly, Python 2.7.2 must be installed. Install it?" /SD IDYES IDNO endPython
    ; DL from http://www.python.org/ftp/python/2.7.2/python-2.7.2.amd64.msi
    File "Prereqs\python-2.7.2.amd64.msi"
    ExecWait '"msiexec" /i "Prereqs\python-2.7.2.amd64.msi"'
  endPython:
  MessageBox MB_YESNO "For OpenNERO to run correctly, wxPython must be installed. Install it?" /SD IDYES IDNO endWxPython
    File "Prereqs\wxPython2.8-win64-unicode-2.8.12.1-py27.exe"
    ExecWait "Prereqs\wxPython2.8-win64-unicode-2.8.12.1-py27.exe"
  endWxPython:
SectionEnd
