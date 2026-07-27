# Coding style

Formatting is defined by the repository `.clang-format`; run clang-format on
changed files before committing. CI verifies it.

## Header layout

Inside a header, declarations come in this order:

1. Types. A class body contains only function declarations, not definitions,
   unless they're one-liners.
2. Free function declarations.
3. Function bodies (an `// Implementation ====...` section; section comments
   are padded with `=` to the end of the line).

## Member order

Class members are grouped `public`, then `protected`, then `private`. Within
each access section the order is: types, then functions, then variables.

## Naming

All types and type aliases use CamelCase (`Rotation`, `TransformSpec`,
`Transformations`). Variables and function names use snake_case (`to_matrix`,
`closest_point_to_origin`).

Non-public member variables end with a trailing underscore (`matrix_`,
`closest_`).