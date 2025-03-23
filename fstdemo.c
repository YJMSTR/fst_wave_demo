#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/libfst/fstapi.h"

int main(int argc, char *argv[])
{
    void *ctx;
    fstHandle clk, rst_n, data, valid, ready;
    const char *filename = "waveform.fst";
    
    // Create FST context and initialize
    ctx = fstWriterCreate(filename, 1);
    if (!ctx) {
        fprintf(stderr, "Failed to create FST file: %s\n", filename);
        return 1;
    }
    
    // Set FST file properties
    fstWriterSetPackType(ctx, FST_WR_PT_LZ4);    // Use LZ4 compression
    fstWriterSetRepackOnClose(ctx, 1);           // Repack on close
    fstWriterSetTimescale(ctx, -9);              // Time unit: ns (10^-9)
    fstWriterSetTimezero(ctx, 0);                // Time zero point
    // Get current time and format it
    time_t current_time = time(NULL);
    char time_str[64];
    struct tm *time_info = localtime(&current_time);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);
    fstWriterSetDate(ctx, time_str);
    fstWriterSetVersion(ctx, "FST Demo v1.0");
    
    // Create hierarchy structure
    fstWriterSetScope(ctx, FST_ST_VCD_MODULE, "top", NULL);
    
    // Create signals
    clk = fstWriterCreateVar(ctx, FST_VT_VCD_WIRE, FST_VD_INPUT, 1, "clk", 0);
    rst_n = fstWriterCreateVar(ctx, FST_VT_VCD_WIRE, FST_VD_INPUT, 1, "rst_n", 0);
    data = fstWriterCreateVar(ctx, FST_VT_VCD_WIRE, FST_VD_OUTPUT, 32, "data", 0);
    valid = fstWriterCreateVar(ctx, FST_VT_VCD_WIRE, FST_VD_OUTPUT, 1, "valid", 0);
    ready = fstWriterCreateVar(ctx, FST_VT_VCD_WIRE, FST_VD_INPUT, 1, "ready", 0);
    
    fstWriterSetUpscope(ctx);  // End current hierarchy
    
    // Initialize signal values
    fstWriterEmitTimeChange(ctx, 0);  // Time 0
    fstWriterEmitValueChange(ctx, clk, "0");
    fstWriterEmitValueChange(ctx, rst_n, "0");
    fstWriterEmitValueChange(ctx, data, "00000000");
    fstWriterEmitValueChange(ctx, valid, "0");
    fstWriterEmitValueChange(ctx, ready, "0");
    
    // Generate waveform data
    uint64_t time;
    char data_str[33];  // 32-bit data + null terminator
    
    // Reset phase
    for (int i = 0; i < 10; i++) {
        time = i * 5;  // 5ns per step
        fstWriterEmitTimeChange(ctx, time);
        fstWriterEmitValueChange(ctx, clk, (i % 2) ? "1" : "0");
        if (i == 8) {
            fstWriterEmitValueChange(ctx, rst_n, "1");  // Release reset
        }
    }
    
    // Normal operation phase
    for (int i = 10; i < 100; i++) {
        time = i * 5;
        fstWriterEmitTimeChange(ctx, time);
        
        // Clock toggle
        fstWriterEmitValueChange(ctx, clk, (i % 2) ? "1" : "0");
        
        // Generate data transfer every 10 cycles
        if (i % 20 == 0 && i > 20) {
            // Convert integer i to 32-bit binary string
            for (int bit = 31; bit >= 0; bit--) {
                data_str[31-bit] = ((i >> bit) & 1) ? '1' : '0';
            }
            data_str[32] = '\0';  // Ensure string is properly terminated
            fstWriterEmitValueChange(ctx, data, data_str);
            fstWriterEmitValueChange(ctx, valid, "1");
            fstWriterEmitValueChange(ctx, ready, "1");
        } else if (i % 20 == 2) {
            fstWriterEmitValueChange(ctx, valid, "0");
            fstWriterEmitValueChange(ctx, ready, "0");
        }
    }
    
    // Close FST file
    fstWriterClose(ctx);
    printf("FST waveform file generated successfully: %s\n", filename);
    
    return 0;
} 