" Vim syntax file
" Language: avdl
" Maintainer: Tom Tsagk
" Latest Revision: 15 September 2019

if exists("b:current_syntax")
  finish
endif

" keywords
syntax keyword avdlKeywords if echo def group
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

" primitive variable types
syntax keyword avdlPrimitiveVariableTypes int float string char

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
