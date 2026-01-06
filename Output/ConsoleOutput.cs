// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

namespace DialScript.Output;

public static class ConsoleOutput
{
    // ANSI codes
    private const string Reset = "\x1b[0m";
    private const string Bold = "\x1b[1m";
    private const string Dim = "\x1b[2m";
    
    private const string Red = "\x1b[31m";
    private const string Green = "\x1b[32m";
    private const string Yellow = "\x1b[33m";
    private const string Cyan = "\x1b[36m";
    private const string White = "\x1b[37m";
    private const string Magenta = "\x1b[35m";
    private const string Gray = "\x1b[90m";
    
    private const string BoldRed = "\x1b[1;31m";
    private const string BoldGreen = "\x1b[1;32m";
    private const string BoldCyan = "\x1b[1;36m";
    private const string BoldWhite = "\x1b[1;37m";
    private const string BoldMagenta = "\x1b[1;35m";

    public static void PrintHeader(string filePath)
    {
        Console.WriteLine($"{BoldCyan}Compiling:{Reset} {filePath}");
    }
    
    public static void PrintFooter(int totalLines, int errorCount)
    {
        if (errorCount == 0)
        {
            Console.WriteLine($"{BoldGreen}Parsing completed:{Reset} {totalLines} lines processed");
        }
        else
        {
            Console.WriteLine($"{BoldRed}Parsing broken:{Reset} {totalLines} lines processed, {errorCount} error(s)");
        }
    }

    public static void PrintEmptyLine(int lineNumber)
    {
        Console.WriteLine($"{Gray}{lineNumber,4} │ {Reset}");
    }
    
    public static void PrintComment(int lineNumber, string text)
    {
        Console.WriteLine($"{Gray}{lineNumber,4} │{Dim} –{text}{Reset}");
    }
    
    public static void PrintScene(int lineNumber, int sceneNumber)
    {
        Console.WriteLine($"{BoldCyan}{lineNumber,4} │ ◉ Scene {sceneNumber}{Reset}");
    }

    public static void PrintDialog(int lineNumber, int dialogNumber)
    {
        Console.WriteLine($"{BoldMagenta}{lineNumber,4} │ ◆ Dialog {dialogNumber}{Reset}");
    }
    
    public static void PrintLevel(int lineNumber, string value)
    {
        Console.WriteLine($"{Gray}{lineNumber,4} │   {Cyan}Level:{Reset} {value}");
    }
    
    public static void PrintLocation(int lineNumber, string value)
    {
        Console.WriteLine($"{Gray}{lineNumber,4} │   {Cyan}Location:{Reset} {value}");
    }
    
    public static void PrintCharacters(int lineNumber, string value)
    {
        Console.WriteLine($"{Gray}{lineNumber,4} │   {Cyan}Characters:{Reset} {value}");
    }
    
    public static void PrintDialogLine(int lineNumber, string name, string text, string? metadata = null)
    {
        if (metadata != null)
        {
            Console.WriteLine($"{Gray}{lineNumber,4} │   {BoldWhite}{name}:{Reset} {text} {Yellow}{metadata}{Reset}");
        }
        else
        {
            Console.WriteLine($"{Gray}{lineNumber,4} │   {BoldWhite}{name}:{Reset} {text}");
        }
    }
    
    public static void PrintError(int lineNumber, string message, string? hint = null, 
        string? lineContent = null, int errorPosition = -1)
    {
        // Error message
        Console.WriteLine($"{BoldRed}{lineNumber,4} │ ✗ {message}{Reset}");
        
        // String with that error
        if (!string.IsNullOrEmpty(lineContent))
        {
            Console.WriteLine($"{Gray}     │   {Red}{lineContent}{Reset}");
            
            // Error position
            if (errorPosition >= 0)
            {
                Console.Write($"{Gray}     │   ");
                Console.Write(new string(' ', errorPosition));
                Console.WriteLine($"{BoldRed}^{Reset}");
            }
        }
        
        // Hint
        if (!string.IsNullOrEmpty(hint))
        {
            Console.WriteLine($"{Gray}     │   {Bold}{Gray}Hint:{Reset} {Gray}{hint}{Reset}");
        }
    }

    public static void PrintErrorMessage(string message)
    {
        Console.WriteLine($"{BoldRed}Error:{Reset} {message}");
    }
    
    public static void PrintVersion(string version)
    {
        Console.WriteLine($"{BoldCyan}DialScript v{version}{Reset}");
    }

    public static void PrintHelp(string version)
    {
        Console.WriteLine($"{BoldCyan}DialScript v{version}{Reset}");
        Console.WriteLine($"{BoldWhite}Usage:{Reset} dialscript <filename.ds> [options]");
        Console.WriteLine();
        Console.WriteLine($"{BoldWhite}Options:{Reset}");
        Console.WriteLine($"  {BoldGreen}--verbose{Reset}    Enable verbose mode");
        Console.WriteLine($"  {BoldGreen}--help{Reset}       Show this help message");
        Console.WriteLine($"  {BoldGreen}--version{Reset}    Show version number");
        Console.WriteLine($"  {BoldGreen}--example{Reset}    Show example .ds file");
    }
    
    public static void PrintExample()
    {
        Console.WriteLine($"{BoldCyan}Example .ds file:{Reset}");
        Console.WriteLine();
        Console.WriteLine($"{BoldCyan}[Scene.1]{Reset}");
        Console.WriteLine($"{Cyan}Level:{Reset} 1");
        Console.WriteLine($"{Cyan}Location:{Reset} Forest");
        Console.WriteLine($"{Cyan}Characters:{Reset} Alan, Beth");
        Console.WriteLine();
        Console.WriteLine($"{Gray}// Main dialog{Reset}");
        Console.WriteLine($"{BoldMagenta}[Dialog.1]{Reset}");
        Console.WriteLine($"{BoldWhite}Alan:{Reset} Hello there! {Yellow}{{Emotion: happy}}{Reset}");
        Console.WriteLine($"{BoldWhite}Beth:{Reset} Hi Alan, nice to see you.");
        Console.WriteLine($"{BoldWhite}Alan:{Reset} Want to go for a walk?");
        Console.WriteLine($"{BoldWhite}Beth:{Reset} Sure! {Yellow}{{Choices: 1, 2}}{Reset}");
        Console.WriteLine($"{BoldWhite}Alan:{Reset} Great, let's go! {Yellow}{{Choice: 1}}{Reset}");
        Console.WriteLine($"{BoldWhite}Alan:{Reset} Maybe next time then. {Yellow}{{Choice: 2}}{Reset}");
    }
}
