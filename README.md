## 1. JSON parser

Added JSON parser implementation.

### Build

```
g++ -std=c++11 parser.cpp -o json-parse
```

### Running

```
cat sample.json | json-parse
```

### What it does

Right now only prints an AST of the JSON, based on the following grammar

## Scope

Probably would add a subset of JS features to this, to do some basic database level stuff.

## Testing

Currently only tested on unix based systems.
