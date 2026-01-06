namespace DialScript.Models;

public enum LineType
{
    // Ignored lines by complier
    Empty,                       // Empty line
    Comment,                     // Comment line

    // Header lines
    Scene,                       // Scene header line
    DialogHeader,                // Dialog header line
    Level,                       // Level header line
    Location,                    // Location header line
    Characters,                  // Characters header line

    // Dialog line
    Dialog,                      // Dialog line

    // Unknown line
    Unknown,                     // Unknown line type

    // Errors
    ErrorEmptyName,              // Empty name
    ErrorEmptyText,              // Empty text
    ErrorNoSpaceAfterColon,      // Missing space after colon
    ErrorInvalidDialogFormat,    // Invalid dialog line format
    ErrorMissingColon,           // Missing colon in dialog
    ErrorUnknownCharacter,       // Unknown character name
    ErrorUnclosedBracket,        // Unclosed bracket
    ErrorMetaNotAtEnd,           // Metadata not at the end of the line
    ErrorTypoScene,              // Did you mean [Scene.N]?
    ErrorTypoDialog,             // Did you mean [Dialog.N]?
    ErrorTypoLevel,              // Did you mean 'Level:'?
    ErrorTypoLocation,           // Did you mean 'Location:'?
    ErrorTypoCharacters,         // Did you mean 'Characters:'?
    ErrorExtraSpaceInHeader,     // Extra space in header
    ErrorExtraSpaceInMetadata,   // Extra space before ':'
    ErrorLeadingSpace            // Leading space in dialog line
}
