file(GLOB MIDI_FILES *.mid)
file(GLOB KAR_FILES *.kar)
install( FILES ${MIDI_FILES} ${KAR_FILES}
         DESTINATION ${DATA_INSTALL_DIR}/kmid )
