include ./define.makefile

.PHONY:all

all:${GAME_DATA_PATH}/mallmall.bin ${GAME_DATA_PATH}/mallmalltype.bin 

${GAME_DATA_PATH}/mallmall.bin:${PROTOCGEN_FILE_PATH}/mall.proto.ds ${RESDB_EXCELMMO_PATH}/mall.xlsx
	${EXCEL2BIN_MMO} --excel=${RESDB_EXCELMMO_PATH}/mall.xlsx  --proto_ds=${PROTOCGEN_FILE_PATH}/mall.proto.ds --proto_package=proto_ff \
		--proto_sheet_msgname=Sheet_mallmall  --excel_sheetname=mall  --proto_msgname=mallmall  --start_row=4 --out_path=${GAME_DATA_PATH}/

${GAME_DATA_PATH}/mallmalltype.bin:${PROTOCGEN_FILE_PATH}/mall.proto.ds ${RESDB_EXCELMMO_PATH}/mall.xlsx
	${EXCEL2BIN_MMO} --excel=${RESDB_EXCELMMO_PATH}/mall.xlsx  --proto_ds=${PROTOCGEN_FILE_PATH}/mall.proto.ds --proto_package=proto_ff \
		--proto_sheet_msgname=Sheet_mallmalltype  --excel_sheetname=malltype  --proto_msgname=mallmalltype  --start_row=4 --out_path=${GAME_DATA_PATH}/
