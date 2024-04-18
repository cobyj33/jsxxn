# jsxxn

A **hobby** JSON parser for C++ 17 - built with the STL
Library and based on the [RFC 8259](https://www.rfc-editor.org/rfc/rfc8259)
JSON standard.

I wanted to create a json parser that would be well-documented for others to
learn from, mostly targeted toward people that have never written code to parse
data before.

> Note that there are much more tested C++ JSON parsers out there!
> * [simdjson/simdjson](https://github.com/simdjson/simdjson.git)
> * [nlohmann/json](https://github.com/nlohmann/json)
> * [Tencent/rapidjson](https://github.com/Tencent/rapidjson/)
> * [open-source-parsers/jsoncpp](https://github.com/open-source-parsers/jsoncpp)
> jsxxn one was created mostly as a challenge and a learning experience!

While jsxxn does work with reasonable data and it ostensibly works with
unreasonable data, and it does provide the correct result for all tests in the
test/data directory, not every edge case has been fuzzed or tested to a 100%
confidence degree as of yet. (or fuzzed at all for that matter)

Example usage can be found in the examples/ directory

## Resources

### Resources - Standards, Grammars, and Descriptions

[json.org](https://www.json.org/json-en.html)

[RFC 8259](https://www.rfc-editor.org/rfc/rfc8259)

[ECMA 404 JSON Standard](https://ecma-international.org/publications-and-standards/standards/ecma-404/)

[RFC 4627](https://www.ietf.org/rfc/rfc4627.txt)

### Resources - Blog Posts

[Parsing JSON is a Minefield - Nicolas Seriot](https://seriot.ch/projects/parsing_json.html)

### Resources - Test Suites

[nst/JSONTestSuite](https://github.com/nst/JSONTestSuite)

[JSXXN_checker](https://json.org/JSXXN_checker/)


