"# debug-tool" 

**1. This tool can collect the content of the .log and regenerate new logs based on keywords**

**2. get the info from Config.ini**

**3. ini info has [target_words] [input] [out]**

    a. [target_words]
    
    "Winter" "Karina"
    
    "Tzuyu"
    
    //("Winter"&&"Karina") || ("Tzuyu")
    AND target words is implement in a vector<CString>
    OR target words is implement in a vector<vector<CString>>
    
    b. [input]
    
    InputLocation = "Default"
    
    InputLocation is optional. if not set or Default, would show the file window for choosing
    
    InputLocation can be fill path or folder path
    
    c. [output]
    
    OutputLocation = ""
    
    Size = 50
    
    Concentrate = 1
    
    Size unit is MB. if output size is over 50MB, would Split to two file.
    
    Concentrate is true, all log would concentrate to 1 log, ignore the size limit
    
  
