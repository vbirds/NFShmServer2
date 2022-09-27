include ./define.makefile

.PHONY:all

all:${GAME_DATA_PATH}/rechargeReturnrechargeReturn.bin ${GAME_DATA_PATH}/rechargeReturntime.bin ${GAME_DATA_PATH}/rechargeReturncontinuousReturn.bin ${GAME_DATA_PATH}/rechargeReturnrechargeTrigger.bin ${GAME_DATA_PATH}/rechargeReturnconstant.bin 

${GAME_DATA_PATH}/rechargeReturnrechargeReturn.bin:${PROTOCGEN_FILE_PATH}/rechargeReturn.proto.ds ${RESDB_EXCELMMO_PATH}/rechargeReturn.xlsx
	${EXCEL2BIN_MMO} --excel=${RESDB_EXCELMMO_PATH}/rechargeReturn.xlsx  --proto_ds=${PROTOCGEN_FILE_PATH}/rechargeReturn.proto.ds --proto_package=proto_ff \
		--proto_sheet_msgname=Sheet_rechargeReturnrechargeReturn  --excel_sheetname=rechargeReturn  --proto_msgname=rechargeReturnrechargeReturn  --start_row=4 --out_path=${GAME_DATA_PATH}/

${GAME_DATA_PATH}/rechargeReturntime.bin:${PROTOCGEN_FILE_PATH}/rechargeReturn.proto.ds ${RESDB_EXCELMMO_PATH}/rechargeReturn.xlsx
	${EXCEL2BIN_MMO} --excel=${RESDB_EXCELMMO_PATH}/rechargeReturn.xlsx  --proto_ds=${PROTOCGEN_FILE_PATH}/rechargeReturn.proto.ds --proto_package=proto_ff \
		--proto_sheet_msgname=Sheet_rechargeReturntime  --excel_sheetname=time  --proto_msgname=rechargeReturntime  --start_row=4 --out_path=${GAME_DATA_PATH}/

${GAME_DATA_PATH}/rechargeReturncontinuousReturn.bin:${PROTOCGEN_FILE_PATH}/rechargeReturn.proto.ds ${RESDB_EXCELMMO_PATH}/rechargeReturn.xlsx
	${EXCEL2BIN_MMO} --excel=${RESDB_EXCELMMO_PATH}/rechargeReturn.xlsx  --proto_ds=${PROTOCGEN_FILE_PATH}/rechargeReturn.proto.ds --proto_package=proto_ff \
		--proto_sheet_msgname=Sheet_rechargeReturncontinuousReturn  --excel_sheetname=continuousReturn  --proto_msgname=rechargeReturncontinuousReturn  --start_row=4 --out_path=${GAME_DATA_PATH}/

${GAME_DATA_PATH}/rechargeReturnrechargeTrigger.bin:${PROTOCGEN_FILE_PATH}/rechargeReturn.proto.ds ${RESDB_EXCELMMO_PATH}/rechargeReturn.xlsx
	${EXCEL2BIN_MMO} --excel=${RESDB_EXCELMMO_PATH}/rechargeReturn.xlsx  --proto_ds=${PROTOCGEN_FILE_PATH}/rechargeReturn.proto.ds --proto_package=proto_ff \
		--proto_sheet_msgname=Sheet_rechargeReturnrechargeTrigger  --excel_sheetname=rechargeTrigger  --proto_msgname=rechargeReturnrechargeTrigger  --start_row=4 --out_path=${GAME_DATA_PATH}/

${GAME_DATA_PATH}/rechargeReturnconstant.bin:${PROTOCGEN_FILE_PATH}/rechargeReturn.proto.ds ${RESDB_EXCELMMO_PATH}/rechargeReturn.xlsx
	${EXCEL2BIN_MMO} --excel=${RESDB_EXCELMMO_PATH}/rechargeReturn.xlsx  --proto_ds=${PROTOCGEN_FILE_PATH}/rechargeReturn.proto.ds --proto_package=proto_ff \
		--proto_sheet_msgname=Sheet_rechargeReturnconstant  --excel_sheetname=constant  --proto_msgname=rechargeReturnconstant  --start_row=4 --out_path=${GAME_DATA_PATH}/
