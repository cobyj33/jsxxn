# Testing File Source Metadata

This directory, "test/sources/", will contain various README's, LICENSE files, 
and other details crediting different JSON test file sources. Each source's
metadata will live under a unique directory with that source's name,
or the author's name if the source's name is not clear.

## JSON Test Suite

Some changes are:

n_structure_object_with_comment.json, n_object_trailing_comment.json
n_object_trailing_comment_slash_open.json changed to 
y_structure_object_with_comment.json, y_object_trailing_comment.json and
y_object_trailing_comment_slash_open.json respectively, as this
json implementation simply discards comments.

## JSON_checker

[JSON_checker](https://json.org/JSON_checker/) is a project hosted on json.org
which provides a test suite available with failing and passing tests. 

Two files have been removed, fail1.json and fail18.json.

fail1.json:
```json
"A JSON payload should be an object or array, not a string."
```

fail1.json states that a JSON file cannot be just a string, although this is
false according to
[RFC 8259 Section 2](https://www.rfc-editor.org/rfc/rfc8259#section-2),
which states "A JSON text is a serialized value.  Note that certain previous
specifications of JSON constrained a JSON text to be an object or an array."
This shows how while JSON could be only an object or array, it can now be any
lone literal as well.

fail18.json:
```json
[[[[[[[[[[[[[[[[[[[["Too deep"]]]]]]]]]]]]]]]]]]]]
```

fail18.json was a test made specifically for the nesting depth of the
JSON_checker program. since
[RFC 8259 Section 7](https://www.rfc-editor.org/rfc/rfc8259#section-9)
states "An implementation may set limits on the maximum depth of nesting.", this
is a test for a specific implementation and doesn't help in identifying 
conformance to the actual standarad.