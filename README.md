# csvflat
![build status](https://github.com/mgualdron/csvflat/actions/workflows/c-cpp.yml/badge.svg)

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

## Building csvflat

The following projects have made `csvflat` possible:

- [libcsv](https://github.com/rgamble/libcsv) - Version 3.0.3 of `libcsv` is included with `csvflat`.

Please consider contributing to those projects if you find `csvflat` useful.

Note that this git repository does not include a `configure` script like a 
distribution tarball normally does.  If you don't want to bother with 
installing `autoconf` and `automake`, then download a distribution
[package](https://github.com/mgualdron/csvflat/releases/download/v0.0.1/csvflat-0.0.1.tar.gz)
and run `configure`:

```
./configure
```

If you want the `csvflat` binary installed in your `$HOME/bin`, you should 
run something like:

```
./configure --prefix=$HOME
```

If you're building from a copy of this git repository, you'll need to have 
`autoconf` and `automake` installed on your system, and run the following 
command to generate a `configure` script:

```
autoreconf -i
```

...and subsequently run `configure` as mentioned before.

After `configure` completes successfully, you can do the usual:

```
make
make check
make install
```

## Author

Miguel Gualdron (dev at gualdron.com).
