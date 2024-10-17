" Vim syntax file
" Language: avdl
" Maintainer: Tom Tsagk
" Latest Revision: 29 November 2021

if exists("b:current_syntax")
  finish
endif

" keywords
syntax keyword avdlKeywords if echo def group savefile
	\ enum struct class class_function function return
	\ for break continue asset extern multistring include

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
syntax match avdlKeywords '{'
syntax match avdlKeywords '}'
syntax match avdlKeywords '\.'
syntax match avdlKeywords '\['
syntax match avdlKeywords ']'

" classes
syntax match avdlKeywords 'dd_matrix'
syntax match avdlKeywords 'dd_world'
syntax match avdlKeywords 'avdl_mesh'
syntax match avdlKeywords 'dd_mesh'
syntax match avdlKeywords 'dd_meshColour'
syntax match avdlKeywords 'dd_meshTexture'
syntax match avdlKeywords 'dd_string3d'
syntax match avdlKeywords 'avdl_particle_system'
syntax match avdlKeywords 'avdl_localisation'
syntax match avdlKeywords 'avdl_program'
syntax match avdlKeywords 'dd_translatef'
syntax match avdlKeywords 'dd_rotatef'
syntax match avdlKeywords 'dd_scalef'
syntax match avdlKeywords 'dd_screen_height_get'
syntax match avdlKeywords 'dd_screen_width_get'
syntax match avdlKeywords 'dd_matrix_pop'
syntax match avdlKeywords 'dd_matrix_push'
syntax match avdlKeywords 'avdl_getUniformLocation'
syntax match avdlKeywords 'avdl_setUniform3f'
syntax match avdlKeywords 'dd_math_ease_linear'
syntax match avdlKeywords 'avdl_useProgram'
syntax match avdlKeywords 'dd_math_min'
syntax match avdlKeywords 'dd_math_max'
syntax match avdlKeywords 'DD_PLY'
syntax match avdlKeywords 'AVDL_INPUT_STATE_DOWN'
syntax match avdlKeywords 'AVDL_INPUT_STATE_UP'
syntax match avdlKeywords 'AVDL_INPUT_STATE_MOVE'
syntax match avdlKeywords 'DD_STRING3D_ALIGN_CENTER'
syntax match avdlKeywords 'avdl_clear_depth'
syntax match avdlKeywords 'dd_clearColour'
syntax match avdlKeywords 'dd_world_prepareReady'
syntax match avdlKeywords 'avdl_setUniform1f'
syntax match avdlKeywords 'avdl_exit'
syntax match avdlKeywords 'avdl_rigitbody'
syntax match avdlKeywords 'avdl_collider'
syntax match avdlKeywords 'avdl_collider_aabb'
syntax match avdlKeywords 'avdl_collider_sphere'

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
