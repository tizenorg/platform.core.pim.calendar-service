SET(CAL_DBUS "cal_dbus")

FIND_PROGRAM(GDBUS_CODEGEN NAMES gdbus-codegen)

ADD_CUSTOM_COMMAND(
	OUTPUT dbus
	COMMAND ${GDBUS_CODEGEN} --generate-c-code ${CAL_DBUS}
			--interface-prefix ${INTERFACE_PREFIX}.
			--c-namespace cal
			${CAL_DBUS}.xml
	DEPENDS ${CAL_DBUS}.xml
	COMMENT "Generated cal_dbus.c/h file")

ADD_CUSTOM_TARGET(GENERATED_DBUS_CODE DEPENDS dbus)
# gdbus-codegen  --generate-c-code cal_dbus --interface-prefix org.tizen.calendar_service. --c-namespace cal cal_dbus.xml
