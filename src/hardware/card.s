.set noreorder

.section .text._temp_bu_init
.global _temp_bu_init
.type _temp_bu_init, @function
_temp_bu_init:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x70
