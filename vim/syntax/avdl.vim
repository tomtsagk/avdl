" Vim syntax file
" Language: ddlang
" Maintainer: Tom Tsagk
" Latest Revision: 15 September 2019

if exists("b:current_syntax")
  finish
endif

" constants (numbers and strings)
syn match ddNumber '\d\+'
syn region ddString start='"' end='"'

" comments
syn match ddComment '#.*$'

" colours
let b:current_syntax = "ddlang"

hi def link ddNumber      Constant
hi def link ddString      Constant
hi def link ddComment     Comment
