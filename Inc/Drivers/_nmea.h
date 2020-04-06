/**
 * @file            gps.h
 * @brief           NMEA main file
 */

/*
 * Copyright (c) 2018 Tilen Majerle
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of NMEA parser library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 */
#ifndef NMEA_H_
#define NMEA_H_

/* Includes ------------------------------------------------------------------*/
#include "_dma_ublox.h"

/* Exported constants --------------------------------------------------------*/
/**
 * @brief           Enables `1` or disables `0` `double precision` for floating point
 *                  values such as latitude, longitude, altitude.
 *
 *                  `double` is used as variable type when enabled, `float` when disabled.
 */
#ifndef NMEA_CFG_DOUBLE
#define NMEA_CFG_DOUBLE                      0
#endif

/**
 * @brief           Enables `1` or disables `0` `GGA` statement parsing.
 *
 * @note            This statement must be enabled to parse:
 *                      - Latitude, Longitude, Altitude
 *                      - Number of satellites in use, fix (no fix, NMEA, DNMEA), UTC time
 */
#ifndef NMEA_CFG_STATEMENT_GPGGA
#define NMEA_CFG_STATEMENT_GPGGA             1
#endif

/**
 * @brief           Enables `1` or disables `0` `GSA` statement parsing.
 *
 * @note            This statement must be enabled to parse:
 *                      - Position/Vertical/Horizontal dilution of precision
 *                      - Fix mode (no fix, 2D, 3D fix)
 *                      - IDs of satellites in use
 */
#ifndef NMEA_CFG_STATEMENT_GPGSA
#define NMEA_CFG_STATEMENT_GPGSA             1
#endif

/**
 * @brief           Enables `1` or disables `0` `RMC` statement parsing.
 *
 * @note            This statement must be enabled to parse:
 *                      - Validity of NMEA signal
 *                      - Ground speed in knots and coarse in degrees
 *                      - Magnetic variation
 *                      - UTC date
 */
#ifndef NMEA_CFG_STATEMENT_GPRMC
#define NMEA_CFG_STATEMENT_GPRMC             1
#endif

/**
 * @brief           Enables `1` or disables `0` `GSV` statement parsing.
 *
 * @note            This statement must be enabled to parse:
 *                      - Number of satellites in view
 *                      - Optional details of each satellite in view. See \ref NMEA_CFG_STATEMENT_GPGSV_SAT_DET
 */
#ifndef NMEA_CFG_STATEMENT_GPGSV
#define NMEA_CFG_STATEMENT_GPGSV             0
#endif

/**
 * @brief           Enables `1` or disables `0` detailed parsing of each
 *                  satellite in view for `GSV` statement.
 *
 * @note            When this feature is disabled, only number of "satellites in view" is parsed
 */
#ifndef NMEA_CFG_STATEMENT_GPGSV_SAT_DET
#define NMEA_CFG_STATEMENT_GPGSV_SAT_DET     0
#endif

/**
 * @brief           NMEA float definition, can be either `float` or `double`
 * @note            Check for \ref NMEA_CFG_DOUBLE configuration
 */
#if NMEA_CFG_DOUBLE || __DOXYGEN__
typedef double nmea_float_t;
#else
typedef float nmea_float_t;
#endif

/* Exported struct ----------------------------------------------------------------*/
/**
 * @brief           Satellite descriptor
 */
typedef struct {
  uint8_t num; /*!< Satellite number */
  uint8_t elevation; /*!< Elevation value */
  uint16_t azimuth; /*!< Azimuth in degrees */
  uint8_t snr; /*!< Signal-to-noise ratio */
} nmea_sat_t;

/**
 * @brief           NMEA main structure
 */
typedef struct {
#if NMEA_CFG_STATEMENT_GPGGA || __DOXYGEN__
  /* Information related to GPGGA statement */
  nmea_float_t latitude; /*!< Latitude in units of degrees */
  nmea_float_t longitude; /*!< Longitude in units of degrees */
  nmea_float_t altitude; /*!< Altitude in units of meters */
  nmea_float_t geo_sep; /*!< Geoid separation in units of meters */
  uint8_t sats_in_use; /*!< Number of satellites in use */
  uint8_t fix; /*!< Fix status. `0` = invalid, `1` = NMEA fix, `2` = DNMEA fix, `3` = PPS fix */
  uint8_t hours; /*!< Hours in UTC */
  uint8_t minutes; /*!< Minutes in UTC */
  uint8_t seconds; /*!< Seconds in UTC */
#endif /* NMEA_CFG_STATEMENT_GPGGA || __DOXYGEN__ */

#if NMEA_CFG_STATEMENT_GPGSA || __DOXYGEN__
  /* Information related to GPGSA statement */
  nmea_float_t dop_h; /*!< Dolution of precision, horizontal */
  nmea_float_t dop_v; /*!< Dolution of precision, vertical */
  nmea_float_t dop_p; /*!< Dolution of precision, position */
  uint8_t fix_mode; /*!< Fix mode. `1` = NO fix, `2` = 2D fix, `3` = 3D fix */
  uint8_t satellites_ids[12]; /*!< List of satellite IDs in use. Valid range is `0` to `sats_in_use` */
#endif /* NMEA_CFG_STATEMENT_GPGSA || __DOXYGEN__ */

#if NMEA_CFG_STATEMENT_GPGSV || __DOXYGEN__
    /* Information related to GPGSV statement */
    uint8_t sats_in_view;                       /*!< Number of satellites in view */
#if NMEA_CFG_STATEMENT_GPGSV_SAT_DET || __DOXYGEN__
    nmea_sat_t sats_in_view_desc[12];
#endif
#endif /* NMEA_CFG_STATEMENT_GPGSV || __DOXYGEN__ */

#if NMEA_CFG_STATEMENT_GPRMC || __DOXYGEN__
  /* Information related to GPRMC statement */
  uint8_t is_valid; /*!< NMEA valid status */
  nmea_float_t speed; /*!< Ground speed in knots */
  nmea_float_t coarse; /*!< Ground coarse */
  nmea_float_t variation; /*!< Magnetic variation */
  uint8_t date; /*!< Fix date */
  uint8_t month; /*!< Fix month */
  uint8_t year; /*!< Fix year */
#endif /* NMEA_CFG_STATEMENT_GPRMC || __DOXYGEN__ */

#if !__DOXYGEN__
  struct {
    uint8_t stat; /*!< Statement index */
    char term_str[13]; /*!< Current term in string format */
    uint8_t term_pos; /*!< Current index position in term */
    uint8_t term_num; /*!< Current term number */

    uint8_t star; /*!< Star detected flag */

    uint8_t crc_calc; /*!< Calculated CRC string */

    union {
      uint8_t dummy; /*!< Dummy byte */
#if NMEA_CFG_STATEMENT_GPGGA
      struct {
        nmea_float_t latitude; /*!< NMEA latitude position in degrees */
        nmea_float_t longitude; /*!< NMEA longitude position in degrees */
        nmea_float_t altitude; /*!< NMEA altitude in meters */
        nmea_float_t geo_sep; /*!< Geoid separation in units of meters */
        uint8_t sats_in_use; /*!< Number of satellites currently in use */
        uint8_t fix; /*!< Type of current fix, `0` = Invalid, `1` = NMEA fix, `2` = Differential NMEA fix */
        uint8_t hours; /*!< Current UTC hours */
        uint8_t minutes; /*!< Current UTC minutes */
        uint8_t seconds; /*!< Current UTC seconds */
      } gga; /*!< GPGGA message */
#endif /* NMEA_CFG_STATEMENT_GPGGA */
#if NMEA_CFG_STATEMENT_GPGSA
      struct {
        nmea_float_t dop_h; /*!< Horizontal dilution of precision */
        nmea_float_t dop_v; /*!< Vertical dilution of precision */
        nmea_float_t dop_p; /*!< Position dilution of precision */
        uint8_t fix_mode; /*!< Fix mode, `1` = No fix, `2` = 2D fix, `3` = 3D fix */
        uint8_t satellites_ids[12]; /*!< IDs of satellites currently in use */
      } gsa; /*!< GPGSA message */
#endif /* NMEA_CFG_STATEMENT_GPGSA */
#if NMEA_CFG_STATEMENT_GPGSV
            struct {
                uint8_t sats_in_view;           /*!< Number of stallites in view */
                uint8_t stat_num;               /*!< Satellite line number during parsing GPGSV data */
            } gsv;                              /*!< GPGSV message */
#endif /* NMEA_CFG_STATEMENT_GPGSV */
#if NMEA_CFG_STATEMENT_GPRMC
      struct {
        uint8_t is_valid; /*!< Status whether NMEA status is valid or not */
        uint8_t date; /*!< Current UTF date */
        uint8_t month; /*!< Current UTF month */
        uint8_t year; /*!< Current UTF year */
        nmea_float_t speed; /*!< Current spead over the ground in knots */
        nmea_float_t coarse; /*!< Current coarse made good */
        nmea_float_t variation; /*!< Current magnetic variation in degrees */
      } rmc; /*!< GPRMC message */
#endif /* NMEA_CFG_STATEMENT_GPRMC */
    } data; /*!< Union with data for each information */
  } p; /*!< Structure with private data */
#endif /* !__DOXYGEN__ */
} nmea_t;

/**
 * @brief           Check if current NMEA data contain valid signal
 * @note            \ref NMEA_CFG_STATEMENT_GPRMC must be enabled and `GPRMC` statement must be sent from NMEA receiver
 * @param[in]       _gh: NMEA handle
 * @return          `1` on success, `0` otherwise
 */
#if NMEA_CFG_STATEMENT_GPRMC || __DOXYGEN__
#define nmea_is_valid(_gh)           ((_gh)->is_valid)
#else
#define nmea_is_valid(_gh)           (0)
#endif /* NMEA_CFG_STATEMENT_GPRMC || __DOXYGEN__ */

/* Exported enum ----------------------------------------------------------------*/
/**
 * @brief           List of optional speed transformation from NMEA values (in knots)
 */
typedef enum {
  /* Metric values */
  nmea_speed_kps, /*!< Kilometers per second */
  nmea_speed_kph, /*!< Kilometers per hour */
  nmea_speed_mps, /*!< Meters per second */
  nmea_speed_mpm, /*!< Meters per minute */

  /* Imperial values */
  nmea_speed_mips, /*!< Miles per second */
  nmea_speed_mph, /*!< Miles per hour */
  nmea_speed_fps, /*!< Foots per second */
  nmea_speed_fpm, /*!< Foots per minute */

  /* Optimized for runners/joggers */
  nmea_speed_mpk, /*!< Minutes per kilometer */
  nmea_speed_spk, /*!< Seconds per kilometer */
  nmea_speed_sp100m, /*!< Seconds per 100 meters */
  nmea_speed_mipm, /*!< Minutes per mile */
  nmea_speed_spm, /*!< Seconds per mile */
  nmea_speed_sp100y, /*!< Seconds per 100 yards */

  /* Nautical values */
  nmea_speed_smph, /*!< Sea miles per hour */
} NMEA_SPEED;

/* Public functions prototype ------------------------------------------------*/
uint8_t nmea_init(nmea_t *gh);
uint8_t nmea_process(nmea_t *gh, const void *data, size_t len);
uint8_t nmea_distance_bearing(nmea_float_t las, nmea_float_t los, nmea_float_t lae, nmea_float_t loe, nmea_float_t *d, nmea_float_t *b);
nmea_float_t nmea_to_speed(nmea_float_t sik, NMEA_SPEED ts);

#endif /* NMEA_H_ */
