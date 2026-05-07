# WikiTerm

C-based CLI utility for fetching and rendering plaintext Wikipedia article content via MediaWiki API.

## Dependencies

* **libcurl**: Network transport.
* **cJSON**: JSON serialization and parsing.
* **POSIX environment**: Required for `popen` and `less` integration.

## Compilation
Just run
```bash
make
```

## Usage

```bash
./wiki "Article Title"
```

## Technical Specification

* **Protocol**: MediaWiki Action API (`action=query`).
* **Format**: `prop=extracts` with `explaintext=1` for clean text removal of HTML/Markdown.
* **Network Configuration**:
  * **Connect Timeout**: 30 seconds.
  * **Total Timeout**: 30 seconds.
  * **Redirects**: Follow location enabled.
  * **User-Agent**: `wiki-extract/1.0`.
* **I/O Handling**:
  * Output is piped to `less -R` for paging support.
  * Falls back to `stdout` if `popen` fails.
* **Memory Management**: Manual buffer reallocation via `write_callback` for dynamic API payloads.

## License

MIT
