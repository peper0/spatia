project = "spatia"
author = "spatia contributors"
release = "0.1"

extensions = ["myst_parser"]
source_suffix = {".rst": "restructuredtext", ".md": "markdown"}
myst_enable_extensions = ["colon_fence", "deflist"]

primary_domain = "cpp"
highlight_language = "cpp"
nitpicky = False
nitpick_ignore = [
    ("cpp:identifier", "std::size_t"),
    ("cpp:identifier", "std::array"),
]

html_theme = "furo"
html_title = "spatia"
html_theme_options = {
    "light_css_variables": {
        "color-brand-primary": "#176b87",
        "color-brand-content": "#176b87",
    },
    "dark_css_variables": {
        "color-brand-primary": "#65c3df",
        "color-brand-content": "#65c3df",
    },
}

# index.rst includes the Markdown landing page and adds Sphinx navigation.
exclude_patterns = ["index.md", "_build", "Thumbs.db", ".DS_Store"]
