# DialScript

Experimental dialog scripting language for games.

## Requirements

- [.NET 9.0 SDK](https://dotnet.microsoft.com/download/dotnet/9.0)

## Build and run

### CLI

```bash
# Build
dotnet build

# Run
dotnet run -- tests/test.ds

# Run with verbose output
dotnet run -- tests/test.ds --verbose
```

## Syntax

| Element | Description                        |
|---------|------------------------------------|
| `[Scene.N]` | Scene block header                 |
| `[Dialog.N]` | Dialog block header                |
| `Level`, `Location`, `Characters` | Scene metadata                     |
| `Name: Text` | Dialog line                        |
| `{Key: Value}` | Line metadata                      |
| `// comment` | Comment                            |

## Example

```
[Scene.1]
Level: 1
Location: Forest
Characters: Alan, Beth

[Dialog.1]
Alan: Hello there! {Emotion: happy}
Beth: Hi Alan!
```
