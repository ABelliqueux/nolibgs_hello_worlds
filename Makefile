hello_world:
	$(MAKE) -C hello_world
hello_2pads:
	$(MAKE) -C hello_2pads
hello_cube:
	$(MAKE) -C hello_cube 
hello_cubetex:
	$(MAKE) -C hello_cubetex 
hello_poly_fun:
	$(MAKE) -C hello_poly_fun 
hello_gte_opti:
	$(MAKE) -C hello_gte_opti 
hello_light:
	$(MAKE) -C hello_light 
hello_multivag:
	$(MAKE) -C hello_multivag 
hello_pad:
	$(MAKE) -C hello_pad 
hello_poly:
	$(MAKE) -C hello_poly 
hello_poly_ft:
	$(MAKE) -C hello_poly_ft 
hello_poly_gt:
	$(MAKE) -C hello_poly_gt 
hello_poly_gt_tw:
	$(MAKE) -C hello_poly_gt_tw 
hello_poly_inline:
	$(MAKE) -C hello_poly_inline 
hello_sio:
	$(MAKE) -C hello_sio 
hello_sprt:
	$(MAKE) -C hello_sprt 
hello_tile:
	$(MAKE) -C hello_tile 
hello_vag:
	$(MAKE) -C hello_vag 
hello_cd:
	$(MAKE) -C hello_cd all 
hello_cdda:
	$(MAKE) -C hello_cdda 

clean:
	$(MAKE) -C hello_2pads clean
	$(MAKE) -C hello_cube clean
	$(MAKE) -C hello_cubetex clean
	$(MAKE) -C hello_poly_fun clean
	$(MAKE) -C hello_gte_opti clean
	$(MAKE) -C hello_light clean
	$(MAKE) -C hello_multivag clean
	$(MAKE) -C hello_pad clean
	$(MAKE) -C hello_poly clean
	$(MAKE) -C hello_poly_ft clean
	$(MAKE) -C hello_poly_gt clean
	$(MAKE) -C hello_poly_gt_tw clean
	$(MAKE) -C hello_poly_inline clean
	$(MAKE) -C hello_rsd clean
	$(MAKE) -C hello_sio clean
	$(MAKE) -C hello_sprt clean
	$(MAKE) -C hello_tile clean
	$(MAKE) -C hello_vag clean
	$(MAKE) -C hello_world clean
	$(MAKE) -C hello_cdda clean
	$(MAKE) -C hello_cd cleansub

all:
	$(MAKE) -C hello_2pads 
	$(MAKE) -C hello_cube 
	$(MAKE) -C hello_cubetex 
	$(MAKE) -C hello_poly_fun 
	$(MAKE) -C hello_gte_opti 
	$(MAKE) -C hello_light 
	$(MAKE) -C hello_multivag 
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
	$(MAKE) -C hello_cdda 
	
# declare phony rules
.PHONY: hello_2pads hello_cube hello_cubetex hello_poly_fun hello_gte_opti hello_light hello_multivag hello_pad hello_poly hello_poly_ft hello_poly_gt hello_poly_gt_tw hello_poly_inline hello_sio hello_sprt hello_tile hello_vag hello_world hello_cdda hello_cd\
		clean all
