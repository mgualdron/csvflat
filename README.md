# csvflat

## Synopsis

A trivial command-line program to replace newlines (0x0a) and carriage-returns
(0x0d) with spaces (0x20).

```
csvflat -h

Usage: csvflat [OPTION]... [FILE]...
Output records from a CSV file after replacing any embedded newlines with spaces.
More than one FILE can be specified.

  -d, --delimiter=DELIM  the delimiting character for the input FILE(s)
  -Q, --csv-quote        CSV quoting character (double-quote is the default)
  -h, --help             This help
```

## Build

Type the following within the source directory:

```
make csvflat
```

Move the binary somewhere in your path, like:

```
mv csvflat ~/bin
```
