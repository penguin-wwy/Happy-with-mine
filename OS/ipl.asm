;OS-ipl
;TAB = 4
      ORG		0x7c00

	JMP		entry
	DB		0x90
	DB		"HELLOIPL"		; 启动区的名称，可以是任意字符串（8字节）
	DW		512				; 每个扇区大小512字节
	DB		1				; 簇大小
	DW		1				; FAT起始位置
	DB		2				; FAT的个数，必须为2
	DW		224				; 根目录大小（一般为224）
	DW		2880			; 该磁盘大小（必须2880扇区）
	DB		0xf0			; 磁盘的种类（必须是0xf0）
	DW		9				; FAT的长度（必须是9扇区）
	DW		18				; 1个磁道有几个扇区（必须18）
	DW		2				; 磁头数（必须2）
	DD		0				; 不使用分区必须是0
	DD		2880			; 重写一次磁盘大小
	DB		0,0,0x29		; 意义不明，固定
	DD		0xffffffff		; 卷标号码
	DB		"HELLO-OS   "	; 磁盘名称（11字节）
	DB		"FAT12   "		; 磁盘格式名称
	RESB	18				; 先空出18字节

; 程序核心

entry:
		MOV		AX,0			; 初始化寄存器
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX

; ディスクを読む

		MOV		AX,0x0820
		MOV		ES,AX
		MOV		CH,0			; 柱面0
		MOV		DH,0			; 磁头0
		MOV		CL,2			; 扇区2
readloop:
		MOV		SI,0			; 记录失败次数的寄存器
retry:
		MOV		AH,0x02			; AH=0x02 : 读盘
		MOV		AL,1			; 1个扇区
		MOV		BX,0
		MOV		DL,0x00			; A驱动器
		INT		0x13			; 调用磁盘BIOS
		JNC		next			; 没出错调转
		ADD		SI,1			; SI加1
		CMP		SI,5			; SI和5比较
		JAE		error			; SI >= 5 跳转error
		MOV		AH,0x00
		MOV		DL,0x00			; A驱动器
		INT		0x13			; 重置驱动器
		JMP		retry
next:
		MOV		AX,ES			; 把内存地址后移0x200
		ADD		AX,0x0020
		MOV		ES,AX			; 没有ADD ES,0x020指令，绕个弯
		ADD		CL,1			; CL加1
		CMP		CL,18			; CL与18比较
		JBE		readloop		; CL <= 18 跳转readloop
		MOV		CL,1
		ADD		DH,1
		CMP		DH,2
		JB		readloop		; DH < 2 跳转readloop
		MOV		DH,0
		ADD		CH,1
		CMP		CH,CYLS
		JB		readloop		; CH < CYLS 跳转readloop



		MOV		[0x0ff0],CH		
		JMP		0xc200

error:
		MOV		SI,msg
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; SI加1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; 文字表示
		MOV		BX,15			; 指定字符颜色
		INT		0x10			; 调用显卡BIOS
		JMP		putloop
fin:
		HLT						; 让CPU停止，等待指令
		JMP		fin				; 无限循环
msg:
		DB		0x0a, 0x0a		; 换行2次
		DB		"load error"
		DB		0x0a			; 换行
		DB		0

		RESB	0x7dfe-($-$$)		

		DB		0x55, 0xaa
