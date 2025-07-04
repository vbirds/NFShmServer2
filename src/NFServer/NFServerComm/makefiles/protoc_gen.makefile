include ./define.makefile
.PHONY:all

all:${ALL_PROTOCGEN_FILE}

${PROTOCOL_SS_H} ${PROTOCOL_SS_NANOPB_H} ${PROTOCOL_SS_CPP} ${PROTOCOL_SS_NANOPB_CPP} :${PROTOCOL_SS_XML}
	mkdir -p ${PROTOCGEN_FILE_PATH}
	${PROTOC} $^ --plugin=protoc-gen-nanopb=${NANOPB_PLUGIN} -I${NANOPB_PLUGIN_PROTO} -I${THIRD_PARTY_INC_PATH} -I${PROTOCOL_PATH} -I${PROTOCOL_SS_PATH}\
		--cpp_out=${PROTOCGEN_FILE_PATH}  --nanopb_out=-e.nanopb:${PROTOCGEN_FILE_PATH}
	${FILE_COPY_EXE} --work="filecopy" --src="${PROTOCOL_SS_H} ${PROTOCOL_SS_NANOPB_H} ${PROTOCOL_SS_CPP} ${PROTOCOL_SS_NANOPB_CPP}" --dst=${NEW_PROTOCGEN_FILE_PATH}/



