SECTIONS
{
	.data : {
		submodule_tbl = . ;
		*(.data.slime)
		submodule_tbl_end = . ;
		KHOOK_tbl = . ;
		*(.data.khook)
		KHOOK_tbl_end = . ;
	}
}
