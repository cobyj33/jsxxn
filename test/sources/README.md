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

