hello_2pads:
	$(MAKE) -C hello_2pads
hello_bs:
	$(MAKE) -C hello_bs
hello_cd:
	$(MAKE) -C hello_cd
hello_cdda:
	$(MAKE) -C hello_cdda
hello_cd_exec:
	$(MAKE) -C hello_cd_exec
hello_cube:
	$(MAKE) -C hello_cube
hello_cubetex:
	$(MAKE) -C hello_cubetex
hello_cubetex_stp:
	$(MAKE) -C hello_cubetex_stp
hello_font:
	$(MAKE) -C hello_font
hello_fx:
	$(MAKE) -C hello_fx
hello_gte_opti:
	$(MAKE) -C hello_gte_opti
hello_light:
	$(MAKE) -C hello_light
hello_mod:
	$(MAKE) -C hello_mod
hello_multi_vag:
	$(MAKE) -C hello_multi_vag
hello_multi_xa:
	$(MAKE) -C hello_multi_xa
hello_ovl_exec:
	$(MAKE) -C hello_ovl_exec
hello_pad:
	$(MAKE) -C hello_pad
hello_poly:
	$(MAKE) -C hello_poly
hello_poly_ft:
	$(MAKE) -C hello_poly_ft
hello_poly_fun:
	$(MAKE) -C hello_poly_fun
hello_poly_gt:
	$(MAKE) -C hello_poly_gt
hello_poly_gt_tw:
	$(MAKE) -C hello_poly_gt_tw
hello_poly_inline:
	$(MAKE) -C hello_poly_inline
hello_poly_stp:
	$(MAKE) -C hello_poly_stp
hello_sio:
	$(MAKE) -C hello_sio
hello_sprt:
	$(MAKE) -C hello_sprt
hello_str:
	$(MAKE) -C hello_str
hello_strplay:
	$(MAKE) -C hello_strplay
hello_tile:
	$(MAKE) -C hello_tile
hello_vag:
	$(MAKE) -C hello_vag
hello_world:
	$(MAKE) -C hello_world
hello_xa:
	$(MAKE) -C hello_xa
hello_xa_streaming:
	$(MAKE) -C hello_xa_streaming

clean:
	$(MAKE) -C hello_2pads clean
	$(MAKE) -C hello_cube clean
	$(MAKE) -C hello_cubetex clean
	$(MAKE) -C hello_cubetex_stp clean
	$(MAKE) -C hello_poly_fun clean
	$(MAKE) -C hello_gte_opti clean
	$(MAKE) -C hello_light clean
	$(MAKE) -C hello_multi_vag clean
	$(MAKE) -C hello_pad clean
	$(MAKE) -C hello_poly clean
	$(MAKE) -C hello_poly_ft clean
	$(MAKE) -C hello_poly_stp clean
	$(MAKE) -C hello_poly_gt clean
	$(MAKE) -C hello_poly_gt_tw clean
	$(MAKE) -C hello_poly_inline clean
	$(MAKE) -C hello_rsd clean
	$(MAKE) -C hello_sio clean
	$(MAKE) -C hello_sprt clean
	$(MAKE) -C hello_tile clean
	$(MAKE) -C hello_vag clean
	$(MAKE) -C hello_world clean
	$(MAKE) -C hello_cdda clean cleansub
	$(MAKE) -C hello_cd cleansub
	$(MAKE) -C hello_xa cleansub
	$(MAKE) -C hello_bs cleansub
	$(MAKE) -C hello_str cleansub
	$(MAKE) -C hello_strplay cleansub

all:
	$(MAKE) -C hello_2pads 
	$(MAKE) -C hello_cube 
	$(MAKE) -C hello_cubetex 
	$(MAKE) -C hello_cubetex_stp 
	$(MAKE) -C hello_poly_fun 
	$(MAKE) -C hello_gte_opti 
	$(MAKE) -C hello_light 
	$(MAKE) -C hello_multi_vag 
	$(MAKE) -C hello_pad 
	$(MAKE) -C hello_poly 
	$(MAKE) -C hello_poly_ft 
	$(MAKE) -C hello_poly_gt 
	$(MAKE) -C hello_poly_gt_tw 
	$(MAKE) -C hello_poly_inline 
	$(MAKE) -C hello_sio 
	$(MAKE) -C hello_sprt 
	$(MAKE) -C hello_tile 
	$(MAKE) -C hello_vag 
	$(MAKE) -C hello_world 
	$(MAKE) -C hello_cd all
	$(MAKE) -C hello_cdda all
	$(MAKE) -C hello_xa all
	$(MAKE) -C hello_bs all
	$(MAKE) -C hello_str all
	$(MAKE) -C hello_strplay all
	
# declare phony rules
.PHONY: hello_2pads hello_bs hello_cd hello_cdda hello_cd_exec hello_cube hello_cubetex hello_cubetex_stp hello_font hello_fx hello_gte_opti hello_light hello_mod hello_multi_vag hello_multi_xa hello_ovl_exec hello_pad hello_poly hello_poly_ft hello_poly_fun hello_poly_gt hello_poly_gt_tw hello_poly_inline hello_poly_stp hello_sio hello_sprt hello_str hello_strplay hello_tile hello_vag hello_world hello_xa hello_xa_streaming \
		clean all
