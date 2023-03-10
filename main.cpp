#include "conio.h"

#include <stdio.h>
#include <windows.h>

#include <QtDebug>

#include "TDC4_interface.h"
#include "TimeTagger4_interface.h"

timetagger4_device *device;
timetagger4_init_parameters params;
timetagger4_read_in read_config; // configure read out behavior
timetagger4_static_info static_info;
timetagger4_configuration config;
timetagger4_read_out read_data; // structure with packet pointers for read data

#include "crono_interface.h"

#ifdef WIN64
#pragma comment(lib, "xtdc4_driver_64.lib")
#else
#pragma comment(lib, "xtdc4_driver.lib")
    //#pragma comment(lib,"xtdc4_driver_x64_32.lib")
#endif

double TDC_resolution_LSB_ns;

void TDCCleanUp()
{
    if (!device)
        return;

    timetagger4_stop_capture(device); // shut down packet generation and DMA transfers
    timetagger4_close(device);        // deactivate TDC4HM_lr
    device = 0;
}

bool init_TDC(double TDCRange_ns)
{
    if (device)
        TDCCleanUp();

    timetagger4_get_default_init_parameters(&params);
    params.buffer_size[0] = 8 * 1024 * 1024; // use 8 MByte as packet-buffer
    int error_code;
    const char *err_message;
    device = timetagger4_init(&params, &error_code, &err_message);
    if ((error_code != TIMETAGGER4_OK) || (!device)) {
        TDCCleanUp();
        return false;
    }

    timetagger4_get_static_info(device, &static_info); // get info from TDC

    if (!static_info.flash_valid) {
        TDCCleanUp();
        return false;
    }

    timetagger4_get_default_configuration(device, &config); // configure the TDC
    error_code = timetagger4_configure(device, &config);    // write configuration to board

    for (__int32 i = 0; i < 4; i++) // set config of the 4 TDC channels
    {
        config.channel[i].enabled = true;  // enable recording hits on this TDC channel
        config.channel[i].rising = false;  // measure falling edge
        config.trigger[i + 1].falling = 1; // measure falling edge
        config.trigger[i + 1].rising = 0;  // measure falling edge
    }

    config.start_rising = false; // record falling edges on the start
    config.trigger[0].falling = 1;
    config.trigger[0].rising = 0;

    timetagger4_param_info parinfo;
    timetagger4_get_param_info(device, &parinfo);

    TDC_resolution_LSB_ns = parinfo.binsize * 0.001;

    for (__int32 i = 0; i < 4; i++) // set config of the 4 TDC channels
    {
        config.channel[i].start = 0; // group range begins right after start pulse
        config.channel[i].stop = __int32(TDCRange_ns
                                         / TDC_resolution_LSB_ns); // recording window end. 1
                                                                   // unit = 500ps or 1000ps
    }

    config.dc_offset[0] = -0.3;     // trigger   threshold level in Volts
    config.dc_offset[0 + 1] = -0.3; // channel 0 threshold level in Volts
    config.dc_offset[1 + 1] = -0.3; // channel 1 threshold level in Volts
    config.dc_offset[2 + 1] = -0.3; // channel 2 threshold level in Volts
    config.dc_offset[3 + 1] = -0.3; // channel 3 threshold level in Volts

    error_code = timetagger4_configure(device, &config); // write configuration to board
    timetagger4_get_param_info(device, &parinfo);

    if (error_code != CRONO_OK)
        return false;

    return true;
}

int main(int argc, char *argv[])
{
    FILE *fo = 0;

    device = 0;
    if (!init_TDC(100000.)) // in this example 100 us TDC range
        return 1;

    printf("waiting for signals\n group range is 100000ns.\n(press any key to "
           "exit)\n");

    read_config.acknowledge_last_read
        = 1; // automatically acknowledge all data as processed on the next call to
             // xtdc4_read(). Old packet pointers are invalid after calling
             // xtdc4_read()
    crono_packet *p = 0;

    double coarse_timestamp_to_ms = 0.5e-6;

    TDC_resolution_LSB_ns = 0.5;
    __int32 status = timetagger4_start_capture(device); // start data capture

    if (status != CRONO_OK) {
        timetagger4_close(device);
        return 1;
    }

    fo = fopen("data.bin", "wb");

    __int32 counter = 0;
    uint32_t n_pack = 0; // Number of packets created
    uint32_t n_ph_tot = 0;
    __int32 limit = 100;

    while (true) {
        counter++;
        if (counter > limit) {
            counter = 0;
            static __int64 last_time = 0;
            __int64 time = GetTickCount64();
            __int64 diff = time - last_time;
            if (diff < 5)
                limit *= 2;
            if (diff > 1000 && limit > 999)
                limit /= 2;

            if (diff > 1000) {
                last_time = time;
                printf("read: no data\n");
                if (_kbhit()) {
                    qDebug() << "final n_pack" << n_pack;
                    while (_kbhit()) {
                        _getch();
                    }
                    break;
                }
            }
        }

        __int32 status = timetagger4_read(device,
                                          &read_config,
                                          &read_data); // get the pointers of the acquired packets

        if (status == CRONO_OK) {
            p = (crono_packet *) read_data.first_packet;
        }

        if (status == CRONO_READ_NO_DATA) {
            continue;
        }

        if (status == CRONO_READ_INTERNAL_ERROR) {
            printf("read: internal error\n");
            continue;
        }

        if (status == CRONO_READ_TIMEOUT) {
            printf("read: timeout\n");
            continue;
        }

        while (p) {
            n_pack++;
            if (n_pack % 1000 == 0) {
                qDebug() << n_pack;
            }
            size_t n_photons = 2 * (crono_packet_data_length(p))
                               - (p->flags & CRONO_PACKET_FLAG_SHORTENED);
            qDebug() << n_photons;

            fwrite(p, sizeof(crono_packet) - 8, 1, fo);

            fwrite((uint32_t *) p->data, 4, n_photons, fo);

            if (p == read_data.last_packet)
                p = 0;
            else {
                p = (crono_packet *) crono_next_packet(p); // go to next packet
            }
        }
    }

    if (device) // stop data acquisition
        timetagger4_stop_capture(device);

    if (fo) {
        fclose(fo);
        fo = 0;
    }

    TDCCleanUp();
    return 0;
}
