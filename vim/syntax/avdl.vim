" Vim syntax file
" Language: avdl
" Maintainer: Tom Tsagk
" Latest Revision: 29 November 2021

if exists("b:current_syntax")
  finish
endif

" keywords
syntax keyword avdlKeywords if echo def group savefile
	\ class class_function function return
	\ for asset extern multistring include

syntax match avdlKeywords '='
syntax match avdlKeywords '+'
syntax match avdlKeywords '-'
syntax match avdlKeywords '*'
syntax match avdlKeywords '/'
syntax match avdlKeywords '%'
syntax match avdlKeywords '>='
syntax match avdlKeywords '=='
syntax match avdlKeywords '<='
syntax match avdlKeywords '&&'
syntax match avdlKeywords '||'
syntax match avdlKeywords '<'
syntax match avdlKeywords '>'
syntax match avdlKeywords "this"
syntax match avdlKeywords '('
syntax match avdlKeywords ')'

" classes
syntax match avdlKeywords 'dd_matrix'
syntax match avdlKeywords 'dd_world'
syntax match avdlKeywords 'dd_mesh'
syntax match avdlKeywords 'dd_meshColour'
syntax match avdlKeywords 'dd_meshTexture'
syntax match avdlKeywords 'dd_string3d'
syntax match avdlKeywords 'avdl_particle_system'
syntax match avdlKeywords 'avdl_localisation'
syntax match avdlKeywords 'avdl_program'

" primitive variable types
syntax keyword avdlPrimitiveVariableTypes int float string char void

" constants (numbers and strings)
syn match ddNumber '[-+]\?\<\d\+\>'
syn match ddFloat '[-+]\?\<\d\+\.\?\d\+\>'
syn region ddString start='"' end='"'

" comments
syn match ddComment '#.*$'

" colours
let b:current_syntax = "avdl"

hi def link ddNumber      Constant
hi def link ddFloat       Constant
hi def link ddString      Constant
hi def link ddComment     Comment
hi def link avdlKeywords  Keyword
hi def link avdlPrimitiveVariableTypes  Keyword
