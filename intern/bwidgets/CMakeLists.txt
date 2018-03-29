set(INC
	.
	generics
	styles
	widgets
)

set(SRC
	bwPainter.cc
	bwStyle.cc
	bwStyleCSS.cc
	bwStyleManager.cc
	bwStyleProperties.cc
	bwWidgetBaseStyle.cc
	styles/bwStyleClassic.cc
	styles/bwStyleFlatGrey.cc
	styles/bwStyleFlatDark.cc
	styles/bwStyleFlatLight.cc

	bwPaintEngine.h
	bwPainter.h
	bwStyle.h
	bwStyleCSS.h
	bwStyleManager.h
	bwStyleProperties.h
	bwWidgetBaseStyle.h
	styles/bwStyleClassic.h
	styles/bwStyleFlatGrey.h
	styles/bwStyleFlatDark.h
	styles/bwStyleFlatLight.h
)

set(LIB
	bw_generics
	bw_widgets
)

set(INC_SYS

)

blender_add_lib(bf_intern_bwidgets "${SRC}" "${INC}" "${INC_SYS}")
target_link_libraries(bf_intern_bwidgets ${LIB})

add_subdirectory(generics)
add_subdirectory(widgets)
