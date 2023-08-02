# mediatools
Media information and validation tools for Twibooru.

Like Derpibooru's mediastat and mediathumb, but also builds a shared library (which we use in Ruby FFI.)

## Usage
```
$ mediastat ~/Videos/1580747166137370051742457.webm
7578179 1341 1280 720 447 10
```

From left to right: filesize in bytes, frame count, canvas width, canvas height, time base numerator, time base denominator

```
$ mediathumb ~/Videos/test.webm 30 ~/Pictures/test.png
```

Generates a thumb at the given time offset in test.webm and saves it to test.png. No output indicates no error.
