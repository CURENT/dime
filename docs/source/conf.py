# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))
import dime


# -- Project information -----------------------------------------------------

project = 'DiME'
copyright = '2023, Nicholas Parsly'
author = 'Nicholas Parsly'

# The full version, including alpha/beta/rc tags
release = '2.0.0'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
        'sphinx.ext.autodoc',
        'sphinx.ext.autosummary',
        'sphinx.ext.githubpages',
        'sphinx.ext.intersphinx',
        'sphinx.ext.mathjax',
        'sphinx.ext.doctest',
        'sphinx.ext.todo',
        'sphinx.ext.viewcode',
        'sphinx_panels',
        'sphinx_copybutton',
        'sphinxcontrib.matlab',
        'myst_nb',
]

# Generate the API documentation when building
autosummary_generate = True

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []

master_doc = 'index'


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'pydata_sphinx_theme'

html_theme_options = {
    "use_edit_page_button": True,
}

html_sidebars = {

        '**': [ 'localtoc.html',]
}

html_context = {
    "github_url": "https://github.com",
    "github_user": "nparsly",
    "github_repo": "dime",
    "github_version": "master",
    "doc_path": "docs/source",

}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

htmlhelp_basename = 'dime'

# -- Options for LaTeX output ---------------------------------------------

# latex_elements = {
#     # The paper size ('letterpaper' or 'a4paper').
#     #
#     # 'preamble': r'\DeclareUnicodeCharacter{2588}{-}',
#     'papersize': 'letterpaper',

#     # The font size ('10pt', '11pt' or '12pt').
#     #
#     'pointsize': '11pt',

#     # Additional stuff for the LaTeX preamble.
#     #
#     # 'preamble': '',

#     # Latex figure (float) alignment
#     #
#     # 'figure_align': 'htbp',
# }

# # Grouping the document tree into LaTeX files. List of tuples
# # (source start file, target name, title,
# #  author, documentclass [howto, manual, or own class]).
# latex_documents = [
#     (master_doc, 'dime.tex', 'DiME Manual',
#      'Nicholas Parsly, Jinning Wang', 'manual'),
# ]

# Favorite icon
html_favicon = 'images/curent.ico'

# Disable smartquotes to display double dashes correctly
smartquotes = False

jupyter_execute_notebooks = "off"

# sphinx-panels shouldn't add bootstrap css since the pydata-sphinx-theme
# already loads it
panels_add_bootstrap_css = False