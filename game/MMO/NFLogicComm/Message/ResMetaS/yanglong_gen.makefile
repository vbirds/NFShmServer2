include ./define.makefile

.PHONY:all

all:${PROTOCGEN_FILE_PATH}/module_yanglong_bin

${PROTOCGEN_FILE_PATH}/module_yanglong_bin:${PROTOCGEN_FILE_PATH}/yanglong.proto.ds ${RESDB_EXCELMMO_PATH}/yanglong.xlsx
	mkdir -p ${PROTOCGEN_FILE_PATH}
	rm -rf ${PROTOCGEN_FILE_PATH}/module_yanglong_bin
	${NFEXCELPROCESS} --work="exceltobin" --src=${RESDB_EXCELMMO_PATH}/yanglong.xlsx  --proto_ds=${PROTOCGEN_FILE_PATH}/yanglong.proto.ds --dst=${PROTOCGEN_FILE_PATH}/;
	${FILE_COPY_EXE} --work="filecopy_notexist" --src="${PROTOCGEN_FILE_PATH}/YanglongDescEx.h ${PROTOCGEN_FILE_PATH}/YanglongDescEx.cpp" --dst=${DESC_STORE_EX_PATH}/
	${FILE_COPY_EXE} --work="filecopy" --src="${PROTOCGEN_FILE_PATH}/E_YanglongYanglong.bin" --dst=${GAME_DATA_PATH}/
	${FILE_COPY_EXE} --work="filecopy" --src="${PROTOCGEN_FILE_PATH}/YanglongYanglongDesc.h ${PROTOCGEN_FILE_PATH}/YanglongYanglongDesc.cpp" --dst=${DESC_STORE_PATH}/
	${FILE_COPY_EXE} --work="filecopy" --src="${PROTOCGEN_FILE_PATH}/E_YanglongZadan.bin" --dst=${GAME_DATA_PATH}/
	${FILE_COPY_EXE} --work="filecopy" --src="${PROTOCGEN_FILE_PATH}/YanglongZadanDesc.h ${PROTOCGEN_FILE_PATH}/YanglongZadanDesc.cpp" --dst=${DESC_STORE_PATH}/
	${FILE_COPY_EXE} --work="filecopy" --src="${PROTOCGEN_FILE_PATH}/E_YanglongRanking.bin" --dst=${GAME_DATA_PATH}/
	${FILE_COPY_EXE} --work="filecopy" --src="${PROTOCGEN_FILE_PATH}/YanglongRankingDesc.h ${PROTOCGEN_FILE_PATH}/YanglongRankingDesc.cpp" --dst=${DESC_STORE_PATH}/
	touch ${PROTOCGEN_FILE_PATH}/module_yanglong_bin