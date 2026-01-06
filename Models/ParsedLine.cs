// Copyright © 2025 Arsenii Motorin
// Licensed under the Apache License, Version 2.0
// See: http://www.apache.org/licenses/LICENSE-2.0

namespace DialScript.Models;

public class ParsedLine
{
    public LineType Type { get; set; } = LineType.Unknown;
    
    public int Number { get; set; }
    
    public string? Value { get; set; }
    
    public string? CharacterName { get; set; }
    
    public string? Text { get; set; }

    public string? Metadata { get; set; }
    
    public int LineNumber { get; set; }
    
    public string OriginalContent { get; set; } = string.Empty;

    public int ErrorPosition { get; set; } = -1;
    
    public static ParsedLine Success(LineType type, int lineNumber, string originalContent)
    {
        return new ParsedLine
        {
            Type = type,
            LineNumber = lineNumber,
            OriginalContent = originalContent
        };
    }

    public static ParsedLine Error(LineType errorType, int lineNumber, string originalContent, int errorPosition = -1)
    {
        return new ParsedLine
        {
            Type = errorType,
            LineNumber = lineNumber,
            OriginalContent = originalContent,
            ErrorPosition = errorPosition
        };
    }

    public bool IsError => Type.ToString().StartsWith("Error");
}
