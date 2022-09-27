include ./define.makefile

.PHONY:all

all:${GAME_DATA_PATH}/soloduanwei.bin ${GAME_DATA_PATH}/solorank.bin ${GAME_DATA_PATH}/solopipei.bin ${GAME_DATA_PATH}/soloconstant.bin 

${GAME_DATA_PATH}/soloduanwei.bin:${PROTOCGEN_FILE_PATH}/solo.proto.ds ${RESDB_EXCELMMO_PATH}/solo.xlsx
	${EXCEL2BIN_MMO} --excel=${RESDB_EXCELMMO_PATH}/solo.xlsx  --proto_ds=${PROTOCGEN_FILE_PATH}/solo.proto.ds --proto_package=proto_ff \
		--proto_sheet_msgname=Sheet_soloduanwei  --excel_sheetname=duanwei  --proto_msgname=soloduanwei  --start_row=4 --out_path=${GAME_DATA_PATH}/

${GAME_DATA_PATH}/solorank.bin:${PROTOCGEN_FILE_PATH}/solo.proto.ds ${RESDB_EXCELMMO_PATH}/solo.xlsx
	${EXCEL2BIN_MMO} --excel=${RESDB_EXCELMMO_PATH}/solo.xlsx  --proto_ds=${PROTOCGEN_FILE_PATH}/solo.proto.ds --proto_package=proto_ff \
		--proto_sheet_msgname=Sheet_solorank  --excel_sheetname=rank  --proto_msgname=solorank  --start_row=4 --out_path=${GAME_DATA_PATH}/

${GAME_DATA_PATH}/solopipei.bin:${PROTOCGEN_FILE_PATH}/solo.proto.ds ${RESDB_EXCELMMO_PATH}/solo.xlsx
	${EXCEL2BIN_MMO} --excel=${RESDB_EXCELMMO_PATH}/solo.xlsx  --proto_ds=${PROTOCGEN_FILE_PATH}/solo.proto.ds --proto_package=proto_ff \
		--proto_sheet_msgname=Sheet_solopipei  --excel_sheetname=pipei  --proto_msgname=solopipei  --start_row=4 --out_path=${GAME_DATA_PATH}/

${GAME_DATA_PATH}/soloconstant.bin:${PROTOCGEN_FILE_PATH}/solo.proto.ds ${RESDB_EXCELMMO_PATH}/solo.xlsx
	${EXCEL2BIN_MMO} --excel=${RESDB_EXCELMMO_PATH}/solo.xlsx  --proto_ds=${PROTOCGEN_FILE_PATH}/solo.proto.ds --proto_package=proto_ff \
		--proto_sheet_msgname=Sheet_soloconstant  --excel_sheetname=constant  --proto_msgname=soloconstant  --start_row=4 --out_path=${GAME_DATA_PATH}/
