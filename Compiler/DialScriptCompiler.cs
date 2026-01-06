using DialScript.Models;
using DialScript.Output;
using DialScript.Parsing;

namespace DialScript.Compiler;

public class CompilerSettings
{
    public bool Verbose { get; set; } = false;
}

public class CompileResult
{

    public bool Success => Errors.Count == 0;
    
    public int TotalLines { get; set; }
    
    public List<CompileError> Errors { get; } = new();

    public List<ParsedLine> ParsedLines { get; } = new();
}

public class DialScriptCompiler
{
    private readonly CompilerSettings _settings;
    
    private bool _hasScene;
    private bool _hasLevel;
    private bool _hasLocation;
    private bool _hasCharacters;
    private bool _inDialog;
    private int _currentScene;
    private HashSet<string> _knownCharacters = new();
    
    public DialScriptCompiler(CompilerSettings? settings = null)
    {
        _settings = settings ?? new CompilerSettings();
    }

    public CompileResult Compile(string filePath)
    {
        var result = new CompileResult();
        
        // Check file exists
        if (!File.Exists(filePath))
        {
            ConsoleOutput.PrintErrorMessage($"cannot open file {filePath}. Does it exist?");
            result.Errors.Add(new CompileError
            {
                LineNumber = 0,
                Message = $"File not found: {filePath}"
            });
            return result;
        }
        
        // Read file
        var lines = File.ReadAllLines(filePath);
        result.TotalLines = lines.Length;
        
        if (_settings.Verbose)
        {
            ConsoleOutput.PrintHeader(filePath);
        }
        
        ResetState();
        
        // Parse lines
        for (var i = 0; i < lines.Length; i++)
        {
            var lineNumber = i + 1;
            var line = lines[i];
            
            var parsed = LineParser.Parse(line, lineNumber);
            result.ParsedLines.Add(parsed);
            
            // Check for errors in context
            ValidateLine(parsed, lines, i, result.Errors);
            
            // Print parsed line in verbose mode
            if (_settings.Verbose)
            {
                PrintParsedLine(parsed);
            }
        }
        
        // Check for final requirements
        ValidateFinalRequirements(lines.Length, result.Errors);
        
        // Print summary
        if (_settings.Verbose)
        {
            ConsoleOutput.PrintFooter(result.TotalLines, result.Errors.Count);
        }
        else
        {
            ConsoleOutput.PrintFooter(result.TotalLines, result.Errors.Count);
        }
        
        return result;
    }
    
    private void ResetState()
    {
        _hasScene = false;
        _hasLevel = false;
        _hasLocation = false;
        _hasCharacters = false;
        _inDialog = false;
        _currentScene = 0;
        _knownCharacters.Clear();
    }
    
    private void ValidateLine(ParsedLine parsed, string[] lines, int index, List<CompileError> errors)
    {
        var lineNumber = index + 1;
        var originalLine = lines[index];
        
        switch (parsed.Type)
        {
            // Default lines
            case LineType.Empty:
                // Check for empty lines between dialog lines
                if (_inDialog && index + 1 < lines.Length)
                {
                    var nextParsed = LineParser.Parse(lines[index + 1], lineNumber + 1);
                    if (nextParsed.Type != LineType.DialogHeader && 
                        nextParsed.Type != LineType.Comment &&
                        !nextParsed.Type.ToString().StartsWith("Error"))
                    {
                        AddError(errors, lineNumber, 
                            "Empty line inside dialog block", 
                            "remove empty lines between dialog lines", 
                            originalLine);
                    }
                }
                break;
                
            case LineType.Scene:
                if (_hasScene)
                {
                    AddError(errors, lineNumber, 
                        "Only one [Scene.X] allowed", 
                        "remove extra scene declarations", 
                        originalLine);
                }
                else if (parsed.Number <= 0)
                {
                    AddError(errors, lineNumber, 
                        "Scene number must be > 0", 
                        "use [Scene.1], [Scene.2], etc.", 
                        originalLine, 7);
                }
                else
                {
                    _currentScene = parsed.Number;
                    _inDialog = false;
                    _knownCharacters.Clear();
                    _hasScene = true;
                }
                break;
                
            case LineType.DialogHeader:
                if (_currentScene == 0)
                {
                    AddError(errors, lineNumber, 
                        "Dialog without [Scene.X]", 
                        "add [Scene.1] before this dialog", 
                        originalLine);
                }
                else if (parsed.Number <= 0)
                {
                    AddError(errors, lineNumber, 
                        "Dialog number must be > 0", 
                        "use [Dialog.1], [Dialog.2], etc.", 
                        originalLine, 8);
                }
                else
                {
                    _inDialog = true;
                }
                break;
                
            case LineType.Level:
                if (_currentScene == 0)
                {
                    AddError(errors, lineNumber, 
                        "Level outside scene", 
                        "move Level: x inside [Scene.X] block", 
                        originalLine);
                }
                else if (_inDialog)
                {
                    AddError(errors, lineNumber, 
                        "Level after dialog", 
                        "move Level: x before [Dialog.X]", 
                        originalLine);
                }
                else if (_hasLevel)
                {
                    AddError(errors, lineNumber, 
                        "Duplicate Level", 
                        "remove extra Level definition", 
                        originalLine);
                }
                else
                {
                    _hasLevel = true;
                }
                break;
                
            case LineType.Location:
                if (_currentScene == 0)
                {
                    AddError(errors, lineNumber, 
                        "Location outside scene", 
                        "move Location: x inside [Scene.X] block", 
                        originalLine);
                }
                else if (_inDialog)
                {
                    AddError(errors, lineNumber, 
                        "Location after dialog", 
                        "move Location: x before [Dialog.X]", 
                        originalLine);
                }
                else if (_hasLocation)
                {
                    AddError(errors, lineNumber, 
                        "Duplicate Location", 
                        "remove extra Location definition", 
                        originalLine);
                }
                else
                {
                    _hasLocation = true;
                }
                break;
                
            case LineType.Characters:
                if (_currentScene == 0)
                {
                    AddError(errors, lineNumber, 
                        "Characters outside scene", 
                        "move Characters: inside [Scene.X] block", 
                        originalLine);
                }
                else if (_inDialog)
                {
                    AddError(errors, lineNumber, 
                        "Characters after dialog", 
                        "move Characters: before [Dialog.X]", 
                        originalLine);
                }
                else if (_hasCharacters)
                {
                    AddError(errors, lineNumber, 
                        "Duplicate Characters", 
                        "remove extra Characters definition", 
                        originalLine);
                }
                else
                {
                    // Parse known characters
                    if (!string.IsNullOrEmpty(parsed.Value))
                    {
                        _knownCharacters = parsed.Value
                            .Split(',')
                            .Select(c => c.Trim())
                            .Where(c => !string.IsNullOrEmpty(c))
                            .ToHashSet();
                    }
                    _hasCharacters = true;
                }
                break;
                
            case LineType.Dialog:
                if (!_inDialog)
                {
                    AddError(errors, lineNumber, 
                        "Stray dialog line", 
                        "add [Dialog.1] before this line", 
                        originalLine);
                }
                else
                {
                    // Check for known characters
                    if (_knownCharacters.Count > 0 && 
                        !string.IsNullOrEmpty(parsed.CharacterName) &&
                        !_knownCharacters.Contains(parsed.CharacterName))
                    {
                        AddError(errors, lineNumber, 
                            "Unknown character", 
                            "add this character to Characters", 
                            originalLine);
                    }
                    
                    // Check for missing metadata brace
                    if (!string.IsNullOrEmpty(parsed.Metadata) && !parsed.Metadata.Contains('}'))
                    {
                        var metaPos = originalLine.IndexOf('{');
                        AddError(errors, lineNumber, 
                            "Missing '}' in metadata", 
                            "close metadata with '}'", 
                            originalLine, metaPos);
                    }
                }
                break;
                
            case LineType.Unknown:
                if (_inDialog)
                {
                    AddError(errors, lineNumber, 
                        "Invalid line in dialog", 
                        "use format: Name: Text", 
                        originalLine);
                }
                else
                {
                    AddError(errors, lineNumber, 
                        "Unknown syntax", 
                        "check spelling or use: [Scene.N], [Dialog.N], Name: Text", 
                        originalLine);
                }
                break;
            
            case LineType.ErrorEmptyName:
                AddError(errors, lineNumber, "Empty name before ':'", "add character name, e.g. Alan: Hello", originalLine);
                break;
                
            case LineType.ErrorMissingColon:
                AddError(errors, lineNumber, "Missing ':' in dialog", "use format: Name: Text", originalLine);
                break;
                
            case LineType.ErrorInvalidDialogFormat:
                AddError(errors, lineNumber, "Wrong dialog format", "use format: Name: Text", originalLine);
                break;
                
            case LineType.ErrorTypoScene:
                AddError(errors, lineNumber, "Did you mean [Scene.N]?", "check spelling", originalLine, 1);
                break;
                
            case LineType.ErrorTypoDialog:
                AddError(errors, lineNumber, "Did you mean [Dialog.N]?", "check spelling", originalLine, 1);
                break;
                
            case LineType.ErrorTypoLevel:
                AddError(errors, lineNumber, "Did you mean 'Level:'?", "check spelling", originalLine);
                break;
                
            case LineType.ErrorTypoLocation:
                AddError(errors, lineNumber, "Did you mean 'Location:'?", "check spelling", originalLine);
                break;
                
            case LineType.ErrorTypoCharacters:
                AddError(errors, lineNumber, "Did you mean 'Characters:'?", "check spelling", originalLine);
                break;
                
            case LineType.ErrorUnclosedBracket:
                AddError(errors, lineNumber, "Missing ']'", "close header with ']'", originalLine, originalLine.Length);
                break;
                
            case LineType.ErrorExtraSpaceInHeader:
                AddError(errors, lineNumber, "Extra space in header", "use [Scene.1] or [Dialog.1] without spaces", originalLine);
                break;
                
            case LineType.ErrorExtraSpaceInMetadata:
                AddError(errors, lineNumber, "Extra space before ':'", "use 'Level:', 'Location:', 'Characters:' without spaces", originalLine);
                break;
                
            case LineType.ErrorLeadingSpace:
                AddError(errors, lineNumber, "Leading space in dialog line", "character name must start at the beginning of the line", originalLine);
                break;
                
            case LineType.ErrorNoSpaceAfterColon:
                AddError(errors, lineNumber, "Missing space after ':'", "add a space after the colon, e.g. 'Name: Text'", originalLine);
                break;
                
            case LineType.ErrorEmptyText:
                AddError(errors, lineNumber, "Empty dialog text", "add text after the colon", originalLine);
                break;
        }
    }

    private void ValidateFinalRequirements(int totalLines, List<CompileError> errors)
    {
        if (!_hasScene)
        {
            AddError(errors, totalLines, 
                "Missing [Scene.X]", 
                "add [Scene.1] at the beginning of file");
        }
        if (!_hasLevel)
        {
            AddError(errors, totalLines, 
                "Missing Level", 
                "add 'Level: N' after [Scene.X]");
        }
        if (!_hasLocation)
        {
            AddError(errors, totalLines, 
                "Missing Location", 
                "add 'Location: name' after [Scene.X]");
        }
        if (!_hasCharacters)
        {
            AddError(errors, totalLines, 
                "Missing Characters", 
                "add 'Characters: Name1, Name2' after [Scene.X]");
        }
    }

    private void AddError(List<CompileError> errors, int lineNumber, string message, 
        string? hint = null, string? lineContent = null, int errorPosition = -1)
    {
        errors.Add(new CompileError
        {
            LineNumber = lineNumber,
            Message = message,
            Hint = hint,
            LineContent = lineContent,
            ErrorPosition = errorPosition
        });
        
        ConsoleOutput.PrintError(lineNumber, message, hint, lineContent, errorPosition);
    }

    private void PrintParsedLine(ParsedLine parsed)
    {
        switch (parsed.Type)
        {
            case LineType.Empty:
                ConsoleOutput.PrintEmptyLine(parsed.LineNumber);
                break;
                
            case LineType.Comment:
                ConsoleOutput.PrintComment(parsed.LineNumber, parsed.Value ?? "");
                break;
                
            case LineType.Scene:
                ConsoleOutput.PrintScene(parsed.LineNumber, parsed.Number);
                break;
                
            case LineType.DialogHeader:
                ConsoleOutput.PrintDialog(parsed.LineNumber, parsed.Number);
                break;
                
            case LineType.Level:
                ConsoleOutput.PrintLevel(parsed.LineNumber, parsed.Value ?? "");
                break;
                
            case LineType.Location:
                ConsoleOutput.PrintLocation(parsed.LineNumber, parsed.Value ?? "");
                break;
                
            case LineType.Characters:
                ConsoleOutput.PrintCharacters(parsed.LineNumber, parsed.Value ?? "");
                break;
                
            case LineType.Dialog:
                ConsoleOutput.PrintDialogLine(parsed.LineNumber, 
                    parsed.CharacterName ?? "", 
                    parsed.Text ?? "", 
                    parsed.Metadata);
                break;
        }
    }
}
