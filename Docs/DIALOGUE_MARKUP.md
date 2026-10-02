# Dialogue Performance Markup

Text input can include non-destructive performance directives.

## Examples

```text
[emotion=angry intensity=0.7]
Where have you been?

[pause=0.4]
[blink]

[emotion=sad intensity=0.4]
I was waiting for you.
```

Supported bootstrap directives:

- `[emotion=<name> intensity=<0..1>]`
- `[pause=<seconds>]`
- `[blink]`
- `[gaze=<target>]`
- `[style=<name>]`

The parser keeps dialogue text and directives as separate items.

Future Timeline compilation will convert:
- text -> SpeechFace track
- emotion -> Emotion track
- pause -> timing gap
- blink -> Instant Event track
- gaze -> Gaze/Head track
- style -> Performance/Personality modifiers
