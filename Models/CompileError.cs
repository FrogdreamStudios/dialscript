namespace DialScript.Models;

public class CompileError
{
    public int LineNumber { get; init; }
    
    public string Message { get; init; } = string.Empty;
    
    public string? Hint { get; init; }
    
    public string? LineContent { get; init; }
    
    public int ErrorPosition { get; init; } = -1;
}
