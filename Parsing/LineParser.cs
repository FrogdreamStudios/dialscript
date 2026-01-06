namespace DialScript.Parsing;

using System.Text.RegularExpressions;
using DialScript.Models;

public static partial class LineParser
{
    
    [GeneratedRegex(@"^\[Scene\.(\d+)\]$", RegexOptions.IgnoreCase)]
    private static partial Regex ScenePattern();
    
    [GeneratedRegex(@"^\[Dialog\.(\d+)\]$", RegexOptions.IgnoreCase)]
    private static partial Regex DialogHeaderPattern();
    
    [GeneratedRegex(@"^Level:\s*(.+)$", RegexOptions.IgnoreCase)]
    private static partial Regex LevelPattern();
    
    [GeneratedRegex(@"^Location:\s*(.+)$", RegexOptions.IgnoreCase)]
    private static partial Regex LocationPattern();
    
    [GeneratedRegex(@"^Characters:\s*(.+)$", RegexOptions.IgnoreCase)]
    private static partial Regex CharactersPattern();
    
    [GeneratedRegex(@"^([^:]+):\s+(.+?)(?:\s*(\{[^}]+\}))?$")]
    private static partial Regex DialogPattern();
    
    [GeneratedRegex(@"^\[([^\]]*)\]$")]
    private static partial Regex BracketPattern();
    
    [GeneratedRegex(@"^(scene|scen|scne|sceen)", RegexOptions.IgnoreCase)]
    private static partial Regex SceneTypoPattern();
    
    [GeneratedRegex(@"^(dialog|dialg|dialgo|dailog)", RegexOptions.IgnoreCase)]
    private static partial Regex DialogTypoPattern();
    
    public static ParsedLine Parse(string line, int lineNumber)
    {
        var originalLine = line;
        var trimmedLine = line.Trim();
        
        // Empty line
        if (string.IsNullOrWhiteSpace(trimmedLine))
        {
            return ParsedLine.Success(LineType.Empty, lineNumber, originalLine);
        }
        
        // Comment
        if (trimmedLine.StartsWith("//"))
        {
            return new ParsedLine
            {
                Type = LineType.Comment,
                LineNumber = lineNumber,
                OriginalContent = originalLine,
                Value = trimmedLine[2..].TrimStart()
            };
        }
        
        // Header
        if (trimmedLine.StartsWith('['))
        {
            return ParseHeader(trimmedLine, lineNumber, originalLine);
        }
        
        // Metadata
        var metadataResult = TryParseMetadata(trimmedLine, lineNumber, originalLine);
        if (metadataResult != null)
        {
            return metadataResult;
        }
        
        // Dialog
        return ParseDialogLine(line, lineNumber, originalLine);
    }
    
    private static ParsedLine ParseHeader(string trimmedLine, int lineNumber, string originalLine)
    {
        // Check for unclosed brackets
        if (!trimmedLine.Contains(']'))
        {
            return ParsedLine.Error(LineType.ErrorUnclosedBracket, lineNumber, originalLine, trimmedLine.Length);
        }
        
        // [Scene.N]
        var sceneMatch = ScenePattern().Match(trimmedLine);
        if (sceneMatch.Success)
        {
            var number = int.Parse(sceneMatch.Groups[1].Value);
            if (number <= 0)
            {
                return ParsedLine.Error(LineType.ErrorTypoScene, lineNumber, originalLine, 7);
            }
            
            return new ParsedLine
            {
                Type = LineType.Scene,
                LineNumber = lineNumber,
                OriginalContent = originalLine,
                Number = number
            };
        }
        
        // [Dialog.N]
        var dialogMatch = DialogHeaderPattern().Match(trimmedLine);
        if (dialogMatch.Success)
        {
            var number = int.Parse(dialogMatch.Groups[1].Value);
            if (number <= 0)
            {
                return ParsedLine.Error(LineType.ErrorTypoDialog, lineNumber, originalLine, 8);
            }
            
            return new ParsedLine
            {
                Type = LineType.DialogHeader,
                LineNumber = lineNumber,
                OriginalContent = originalLine,
                Number = number
            };
        }
        
        // Check if user made a typo in header
        var bracketMatch = BracketPattern().Match(trimmedLine);
        if (bracketMatch.Success)
        {
            var content = bracketMatch.Groups[1].Value;
            
            // Check for extra spaces
            if (content.Contains(' '))
            {
                return ParsedLine.Error(LineType.ErrorExtraSpaceInHeader, lineNumber, originalLine);
            }
            
            // Check for missing spaces before ':'
            // TODO: add more typo patterns
            if (SceneTypoPattern().IsMatch(content) || content.StartsWith("scene", StringComparison.OrdinalIgnoreCase))
            {
                return ParsedLine.Error(LineType.ErrorTypoScene, lineNumber, originalLine, 1);
            }
            
            if (DialogTypoPattern().IsMatch(content) || content.StartsWith("dialog", StringComparison.OrdinalIgnoreCase))
            {
                return ParsedLine.Error(LineType.ErrorTypoDialog, lineNumber, originalLine, 1);
            }
        }
        
        return ParsedLine.Error(LineType.ErrorTypoScene, lineNumber, originalLine, 1);
    }
    
    private static ParsedLine? TryParseMetadata(string trimmedLine, int lineNumber, string originalLine)
    {
        // Level
        var levelMatch = LevelPattern().Match(trimmedLine);
        if (levelMatch.Success)
        {
            return new ParsedLine
            {
                Type = LineType.Level,
                LineNumber = lineNumber,
                OriginalContent = originalLine,
                Value = levelMatch.Groups[1].Value.Trim()
            };
        }
        
        // Location
        var locationMatch = LocationPattern().Match(trimmedLine);
        if (locationMatch.Success)
        {
            return new ParsedLine
            {
                Type = LineType.Location,
                LineNumber = lineNumber,
                OriginalContent = originalLine,
                Value = locationMatch.Groups[1].Value.Trim()
            };
        }
        
        // Characters
        var charactersMatch = CharactersPattern().Match(trimmedLine);
        if (charactersMatch.Success)
        {
            return new ParsedLine
            {
                Type = LineType.Characters,
                LineNumber = lineNumber,
                OriginalContent = originalLine,
                Value = charactersMatch.Groups[1].Value.Trim()
            };
        }
        
        // Check for typos in metadata headers
        // TODO: add more typo patterns
        if (trimmedLine.StartsWith("leve", StringComparison.OrdinalIgnoreCase) && 
            trimmedLine.Contains(':') &&
            !trimmedLine.StartsWith("level:", StringComparison.OrdinalIgnoreCase))
        {
            return ParsedLine.Error(LineType.ErrorTypoLevel, lineNumber, originalLine);
        }
        
        if (trimmedLine.StartsWith("locatio", StringComparison.OrdinalIgnoreCase) && 
            trimmedLine.Contains(':') &&
            !trimmedLine.StartsWith("location:", StringComparison.OrdinalIgnoreCase))
        {
            return ParsedLine.Error(LineType.ErrorTypoLocation, lineNumber, originalLine);
        }
        
        if (trimmedLine.StartsWith("character", StringComparison.OrdinalIgnoreCase) && 
            trimmedLine.Contains(':') &&
            !trimmedLine.StartsWith("characters:", StringComparison.OrdinalIgnoreCase))
        {
            return ParsedLine.Error(LineType.ErrorTypoCharacters, lineNumber, originalLine);
        }
        
        return null;
    }
    
    private static ParsedLine ParseDialogLine(string line, int lineNumber, string originalLine)
    {
        var trimmedLine = line.Trim();
        
        // Check for leading spaces
        if (line.Length > 0 && char.IsWhiteSpace(line[0]) && line.Trim().Contains(':'))
        {
            return ParsedLine.Error(LineType.ErrorLeadingSpace, lineNumber, originalLine);
        }
        
        // Check for colon
        var colonIndex = trimmedLine.IndexOf(':');
        if (colonIndex < 0)
        {
            return ParsedLine.Error(LineType.Unknown, lineNumber, originalLine);
        }
        
        // Name of the character
        var name = trimmedLine[..colonIndex].Trim();
        if (string.IsNullOrEmpty(name))
        {
            return ParsedLine.Error(LineType.ErrorEmptyName, lineNumber, originalLine);
        }
        
        // Text after colon
        var afterColon = trimmedLine[(colonIndex + 1)..];
        
        // Check for space after colon
        // TODO: should we remove that?
        if (afterColon.Length > 0 && !char.IsWhiteSpace(afterColon[0]))
        {
            return ParsedLine.Error(LineType.ErrorNoSpaceAfterColon, lineNumber, originalLine);
        }
        
        var textPart = afterColon.Trim();
        if (string.IsNullOrEmpty(textPart))
        {
            return ParsedLine.Error(LineType.ErrorEmptyText, lineNumber, originalLine);
        }
        
        // Metadata
        string? metadata = null;
        string text = textPart;
        
        var metaStart = textPart.IndexOf('{');
        if (metaStart >= 0)
        {
            var metaEnd = textPart.IndexOf('}', metaStart);
            if (metaEnd < 0)
            {
                return ParsedLine.Error(LineType.ErrorUnclosedBracket, lineNumber, originalLine, 
                    originalLine.IndexOf('{'));
            }
            
            metadata = textPart[metaStart..(metaEnd + 1)];
            text = textPart[..metaStart].Trim();
            
            // Check that metadata is at the end of the line
            var afterMeta = textPart[(metaEnd + 1)..].Trim();
            if (!string.IsNullOrEmpty(afterMeta))
            {
                return ParsedLine.Error(LineType.ErrorMetaNotAtEnd, lineNumber, originalLine);
            }
        }
        
        return new ParsedLine
        {
            Type = LineType.Dialog,
            LineNumber = lineNumber,
            OriginalContent = originalLine,
            CharacterName = name,
            Text = text,
            Metadata = metadata
        };
    }
}
