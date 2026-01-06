using DialScript.Compiler;
using DialScript.Output;

namespace DialScript;

public class Program
{
    private const string Version = "0.0.2";
    
    public static int Main(string[] args)
    {
        // If no arguments were passed, print help
        if (args.Length == 0)
        {
            ConsoleOutput.PrintHelp(Version);
            return 0;
        }
        
        // Parse arguments
        var settings = new CompilerSettings();
        string? filename = null;
        
        foreach (var arg in args)
        {
            switch (arg)
            {
                case "--verbose" or "-v":
                    settings.Verbose = true;
                    break;
                    
                case "--help" or "-h":
                    ConsoleOutput.PrintHelp(Version);
                    return 0;
                    
                case "--version":
                    ConsoleOutput.PrintVersion(Version);
                    return 0;
                    
                case "--example":
                    ConsoleOutput.PrintExample();
                    return 0;
                    
                default:
                    if (arg.StartsWith('-'))
                    {
                        ConsoleOutput.PrintErrorMessage($"unknown option '{arg}'");
                        Console.WriteLine("Use 'dialscript --help' for usage information");
                        return 1;
                    }
                    filename = arg;
                    break;
            }
        }
        
        // Check that filename was specified
        if (string.IsNullOrEmpty(filename))
        {
            ConsoleOutput.PrintErrorMessage("no input file specified");
            return 1;
        }
        
        // Check for .ds extension
        if (!filename.EndsWith(".ds"))
        {
            ConsoleOutput.PrintErrorMessage("only .ds files are supported");
            return 1;
        }
        
        // Compile
        var compiler = new DialScriptCompiler(settings);
        var result = compiler.Compile(filename);
        
        // Return error count as exit code
        return result.Errors.Count;
    }
}
