################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
StopWatchMode.obj: ../StopWatchMode.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"/Applications/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.1.LTS/bin/cl430" -vmspx --use_hw_mpy=none --include_path="/Applications/ti/ccsv8/ccs_base/msp430/include" --include_path="/Users/sam/Documents/Electronics/MC Codes/OutOfBox_MSP430FR4133" --include_path="/Users/sam/Documents/Electronics/MC Codes/OutOfBox_MSP430FR4133/driverlib/MSP430FR2xx_4xx" --include_path="/Applications/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.1.LTS/include" --advice:power="all" --advice:hw_config=all --define=__MSP430FR4133__ -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="StopWatchMode.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

TempSensorMode.obj: ../TempSensorMode.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"/Applications/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.1.LTS/bin/cl430" -vmspx --use_hw_mpy=none --include_path="/Applications/ti/ccsv8/ccs_base/msp430/include" --include_path="/Users/sam/Documents/Electronics/MC Codes/OutOfBox_MSP430FR4133" --include_path="/Users/sam/Documents/Electronics/MC Codes/OutOfBox_MSP430FR4133/driverlib/MSP430FR2xx_4xx" --include_path="/Applications/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.1.LTS/include" --advice:power="all" --advice:hw_config=all --define=__MSP430FR4133__ -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="TempSensorMode.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

hal_LCD.obj: ../hal_LCD.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"/Applications/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.1.LTS/bin/cl430" -vmspx --use_hw_mpy=none --include_path="/Applications/ti/ccsv8/ccs_base/msp430/include" --include_path="/Users/sam/Documents/Electronics/MC Codes/OutOfBox_MSP430FR4133" --include_path="/Users/sam/Documents/Electronics/MC Codes/OutOfBox_MSP430FR4133/driverlib/MSP430FR2xx_4xx" --include_path="/Applications/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.1.LTS/include" --advice:power="all" --advice:hw_config=all --define=__MSP430FR4133__ -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="hal_LCD.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"/Applications/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.1.LTS/bin/cl430" -vmspx --use_hw_mpy=none --include_path="/Applications/ti/ccsv8/ccs_base/msp430/include" --include_path="/Users/sam/Documents/Electronics/MC Codes/OutOfBox_MSP430FR4133" --include_path="/Users/sam/Documents/Electronics/MC Codes/OutOfBox_MSP430FR4133/driverlib/MSP430FR2xx_4xx" --include_path="/Applications/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.1.LTS/include" --advice:power="all" --advice:hw_config=all --define=__MSP430FR4133__ -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="main.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


