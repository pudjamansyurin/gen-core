/*
 * _nmea.c
 *
 *  Created on: Aug 19, 2019
 *      Author: Puja
 */

/**
 * \file            nmea.c
 * \brief           NMEA main file
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
#include "_nmea.h"

#define FLT(x)              ((nmea_float_t)(x))
#define D2R(x)              FLT(FLT(x) * FLT(0.01745329251994)) /*!< Degrees to radians */
#define R2D(x)              FLT(FLT(x) * FLT(57.29577951308232))/*!< Radians to degrees */
#define EARTH_RADIUS        FLT(6371.0) /*!< Earth radius in units of kilometers */

#define STAT_UNKNOWN        0
#define STAT_GGA            1
#define STAT_GSA            2
#define STAT_GSV            3
#define STAT_RMC            4

#define CRC_ADD(_nh, ch)    (_nh)->p.crc_calc ^= (uint8_t)(ch)
#define TERM_ADD(_nh, ch)   do {    \
		if ((_nh)->p.term_pos < (sizeof((_nh)->p.term_str) - 1)) {  \
			(_nh)->p.term_str[(_nh)->p.term_pos++] = (ch);  \
			(_nh)->p.term_str[(_nh)->p.term_pos] = 0;   \
		}                               \
} while (0)
#define TERM_NEXT(_nh)      do { (_nh)->p.term_str[((_nh)->p.term_pos = 0)] = 0; (_nh)->p.term_num++; } while (0)

#define CIN(x)              ((x) >= '0' && (x) <= '9')
#define CIHN(x)             (((x) >= '0' && (x) <= '9') || ((x) >= 'a' && (x) <= 'f') || ((x) >= 'A' && (x) <= 'F'))
#define CTN(x)              ((x) - '0')
#define CHTN(x)             (((x) >= '0' && (x) <= '9') ? ((x) - '0') : (((x) >= 'a' && (x) <= 'z') ? ((x) - 'a' + 10) : (((x) >= 'A' && (x) <= 'Z') ? ((x) - 'A' + 10) : 0)))

/**
 * \brief           Parse number as integer
 * \param[in]       gh: NMEA handle
 * \param[in]       t: Text to parse. Set to `NULL` to parse current NMEA term
 * \return          Parsed integer
 */
static int32_t
parse_number(nmea_t *nh, const char *t) {
  int32_t res = 0;
  uint8_t minus;

  if (t == NULL) {
    t = nh->p.term_str;
  }
  for (; t != NULL && *t == ' '; t++) {
  } /* Strip leading spaces */

  minus = (*t == '-' ? (t++, 1) : 0);
  for (; t != NULL && CIN(*t); t++) {
    res = 10 * res + CTN(*t);
  }
  return minus ? -res : res;
}

/**
 * \brief           Parse number as double and convert it to \ref nmea_float_t
 * \param[in]       gh: NMEA handle
 * \param[in]       t: Text to parse. Set to `NULL` to parse current NMEA term
 * \return          Parsed double in \ref nmea_float_t format
 */
static nmea_float_t
parse_float_number(nmea_t *nh, const char *t) {
  nmea_float_t res;

  if (t == NULL) {
    t = nh->p.term_str;
  }
  for (; t != NULL && *t == ' '; t++) {
  } /* Strip leading spaces */

#if NMEA_CFG_DOUBLE
	res = strtod(t, NULL);                      /* Parse string to double */
#else /* NMEA_CFG_DOUBLE */
  res = strtof(t, NULL); /* Parse string to float */
#endif /* !NMEA_CFG_DOUBLE */

  return FLT(res); /* Return casted value, based on float size */
}

/**
 * \brief           Parse latitude/longitude NMEA format to double
 *
 *                  NMEA output for latitude is ddmm.sss and longitude is dddmm.sss
 * \param[in]       gh: NMEA handle
 * \return          Latitude/Longitude value in degrees
 */
static nmea_float_t
parse_lat_long(nmea_t *nh) {
  nmea_float_t ll, deg, min;

  ll = parse_float_number(nh, NULL); /* Parse value as double */
  deg = FLT((int )((int )ll / 100)); /* Get absolute degrees value, interested in integer part only */
  min = ll - (deg * FLT(100)); /* Get remaining part from full number, minutes */
  ll = deg + (min / FLT(60)); /* Calculate latitude/longitude */

  return ll;
}

/**
 * \brief           Parse received term
 * \param[in]       gh: NMEA handle
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
parse_term(nmea_t *nh) {
  if (nh->p.term_num == 0) { /* Check string type */
    if (0) {
#if NMEA_CFG_STATEMENT_GPGGA
    } else if (!strncmp(nh->p.term_str, "$GPGGA", 6) || !strncmp(nh->p.term_str, "$GNGGA", 6)) {
      nh->p.stat = STAT_GGA;
#endif /* NMEA_CFG_STATEMENT_GPGGA */
#if NMEA_CFG_STATEMENT_GPGSA
    } else if (!strncmp(nh->p.term_str, "$GPGSA", 6) || !strncmp(nh->p.term_str, "$GNGSA", 6)) {
      nh->p.stat = STAT_GSA;
#endif /* NMEA_CFG_STATEMENT_GPGSA */
#if NMEA_CFG_STATEMENT_GPGSV
		} else if (!strncmp(nh->p.term_str, "$GPGSV", 6) || !strncmp(nh->p.term_str, "$GNGSV", 6)) {
			nh->p.stat = STAT_GSV;
#endif /* NMEA_CFG_STATEMENT_GPGSV */
#if NMEA_CFG_STATEMENT_GPRMC
    } else if (!strncmp(nh->p.term_str, "$GPRMC", 6) || !strncmp(nh->p.term_str, "$GNRMC", 6)) {
      nh->p.stat = STAT_RMC;
#endif /* NMEA_CFG_STATEMENT_GPRMC */
    } else {
      nh->p.stat = STAT_UNKNOWN; /* Invalid statement for library */
    }
    return 1;
  }

  /* Start parsing terms */
  if (nh->p.stat == STAT_UNKNOWN) {
#if NMEA_CFG_STATEMENT_GPGGA
  } else if (nh->p.stat == STAT_GGA) { /* Process GPGGA statement */
    switch (nh->p.term_num) {
      case 1: /* Process UTC time */
        nh->p.data.gga.hours = 10 * CTN(nh->p.term_str[0]) + CTN(nh->p.term_str[1]);
        nh->p.data.gga.minutes = 10 * CTN(nh->p.term_str[2]) + CTN(nh->p.term_str[3]);
        nh->p.data.gga.seconds = 10 * CTN(nh->p.term_str[4]) + CTN(nh->p.term_str[5]);
        break;
      case 2: /* Latitude */
        nh->p.data.gga.latitude = parse_lat_long(nh); /* Parse latitude */
        break;
      case 3: /* Latitude north/south information */
        if (nh->p.term_str[0] == 'S' || nh->p.term_str[0] == 's') {
          nh->p.data.gga.latitude = -nh->p.data.gga.latitude;
        }
        break;
      case 4: /* Longitude */
        nh->p.data.gga.longitude = parse_lat_long(nh); /* Parse longitude */
        break;
      case 5: /* Longitude east/west information */
        if (nh->p.term_str[0] == 'W' || nh->p.term_str[0] == 'w') {
          nh->p.data.gga.longitude = -nh->p.data.gga.longitude;
        }
        break;
      case 6: /* Fix status */
        nh->p.data.gga.fix = (uint8_t) parse_number(nh, NULL);
        break;
      case 7: /* Satellites in use */
        nh->p.data.gga.sats_in_use = (uint8_t) parse_number(nh, NULL);
        break;
      case 9: /* Altitude */
        nh->p.data.gga.altitude = parse_float_number(nh, NULL);
        break;
      case 11: /* Altitude above ellipsoid */
        nh->p.data.gga.geo_sep = parse_float_number(nh, NULL);
        break;
      default:
        break;
    }
#endif /* NMEA_CFG_STATEMENT_GPGGA */
#if NMEA_CFG_STATEMENT_GPGSA
  } else if (nh->p.stat == STAT_GSA) { /* Process GPGSA statement */
    switch (nh->p.term_num) {
      case 2: /* Process fix mode */
        nh->p.data.gsa.fix_mode = (uint8_t) parse_number(nh, NULL);
        break;
      case 15: /* Process PDOP */
        nh->p.data.gsa.dop_p = parse_float_number(nh, NULL);
        break;
      case 16: /* Process HDOP */
        nh->p.data.gsa.dop_h = parse_float_number(nh, NULL);
        break;
      case 17: /* Process VDOP */
        nh->p.data.gsa.dop_v = parse_float_number(nh, NULL);
        break;
      default:
        /* Parse satellite IDs */
        if (nh->p.term_num >= 3 && nh->p.term_num <= 14) {
          nh->p.data.gsa.satellites_ids[nh->p.term_num - 3] = (uint8_t) parse_number(nh, NULL);
        }
        break;
    }
#endif /* NMEA_CFG_STATEMENT_GPGSA */
#if NMEA_CFG_STATEMENT_GPGSV
	} else if (nh->p.stat == STAT_GSV) { /* Process GPGSV statement */
		switch (nh->p.term_num) {
		case 2: /* Current GPGSV statement number */
			nh->p.data.gsv.stat_num = (uint8_t) parse_number(nh, NULL);
			break;
		case 3: /* Process satellites in view */
			nh->p.data.gsv.sats_in_view = (uint8_t) parse_number(nh, NULL);
			break;
		default:
			#if NMEA_CFG_STATEMENT_GPGSV_SAT_DET
			if (nh->p.term_num >= 4 && nh->p.term_num <= 19) {  /* Check current term number */
				uint8_t index, term_num = nh->p.term_num - 4;   /* Normalize term number from 4-19 to 0-15 */
				uint16_t value;

				index = 4 * (nh->p.data.gsv.stat_num - 1) + term_num / 4;   /* Get array index */
				if (index < sizeof(nh->sats_in_view_desc) / sizeof(nh->sats_in_view_desc[0])) {
					value = (uint16_t)parse_number(nh, NULL);   /* Parse number as integer */
					switch (term_num % 4) {
					case 0: nh->sats_in_view_desc[index].num = value; break;
					case 1: nh->sats_in_view_desc[index].elevation = value; break;
					case 2: nh->sats_in_view_desc[index].azimuth = value; break;
					case 3: nh->sats_in_view_desc[index].snr = value; break;
					default: break;
					}
				}
			}
#endif /* NMEA_CFG_STATEMENT_GPGSV_SAT_DET */
			break;
		}
#endif /* NMEA_CFG_STATEMENT_GPGSV */
#if NMEA_CFG_STATEMENT_GPRMC
  } else if (nh->p.stat == STAT_RMC) { /* Process GPRMC statement */
    switch (nh->p.term_num) {
      case 2: /* Process valid status */
        nh->p.data.rmc.is_valid = (nh->p.term_str[0] == 'A');
        break;
      case 7: /* Process ground speed in knots */
        nh->p.data.rmc.speed = parse_float_number(nh, NULL);
        break;
      case 8: /* Process true ground coarse */
        nh->p.data.rmc.coarse = parse_float_number(nh, NULL);
        break;
      case 9: /* Process date */
        nh->p.data.rmc.date = (uint8_t) (10 * CTN(nh->p.term_str[0]) + CTN(nh->p.term_str[1]));
        nh->p.data.rmc.month = (uint8_t) (10 * CTN(nh->p.term_str[2]) + CTN(nh->p.term_str[3]));
        nh->p.data.rmc.year = (uint8_t) (10 * CTN(nh->p.term_str[4]) + CTN(nh->p.term_str[5]));
        break;
      case 10: /* Process magnetic variation */
        nh->p.data.rmc.variation = parse_float_number(nh, NULL);
        break;
      case 11: /* Process magnetic variation east/west */
        if (nh->p.term_str[0] == 'W' || nh->p.term_str[0] == 'w') {
          nh->p.data.rmc.variation = -nh->p.data.rmc.variation;
        }
        break;
      default:
        break;
    }
#endif /* NMEA_CFG_STATEMENT_GPRMC */
  }
  return 1;
}

/**
 * \brief           Compare calculated CRC with received CRC
 * \param[in]       gh: NMEA handle
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
check_crc(nmea_t *nh) {
  uint8_t crc;
  crc = (uint8_t) ((CHTN(nh->p.term_str[0]) & 0x0F) << 0x04) | (CHTN(nh->p.term_str[1]) & 0x0F); /* Convert received CRC from string (hex) to number */
  return nh->p.crc_calc == crc; /* They must match! */
}

/**
 * \brief           Copy temporary memory to user memory
 * \param[in]       gh: NMEA handle
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
copy_from_tmp_memory(nmea_t *nh) {
  if (0) {
#if NMEA_CFG_STATEMENT_GPGGA
  } else if (nh->p.stat == STAT_GGA) {
    nh->latitude = nh->p.data.gga.latitude;
    nh->longitude = nh->p.data.gga.longitude;
    nh->altitude = nh->p.data.gga.altitude;
    nh->geo_sep = nh->p.data.gga.geo_sep;
    nh->sats_in_use = nh->p.data.gga.sats_in_use;
    nh->fix = nh->p.data.gga.fix;
    nh->hours = nh->p.data.gga.hours;
    nh->minutes = nh->p.data.gga.minutes;
    nh->seconds = nh->p.data.gga.seconds;
#endif /* NMEA_CFG_STATEMENT_GPGGA */
#if NMEA_CFG_STATEMENT_GPGSA
  } else if (nh->p.stat == STAT_GSA) {
    nh->dop_h = nh->p.data.gsa.dop_h;
    nh->dop_p = nh->p.data.gsa.dop_p;
    nh->dop_v = nh->p.data.gsa.dop_v;
    nh->fix_mode = nh->p.data.gsa.fix_mode;
    memcpy(nh->satellites_ids, nh->p.data.gsa.satellites_ids, sizeof(nh->satellites_ids));
#endif /* NMEA_CFG_STATEMENT_GPGSA */
#if NMEA_CFG_STATEMENT_GPGSV
	} else if (nh->p.stat == STAT_GSV) {
		nh->sats_in_view = nh->p.data.gsv.sats_in_view;
#endif /* NMEA_CFG_STATEMENT_GPGSV */
#if NMEA_CFG_STATEMENT_GPRMC
  } else if (nh->p.stat == STAT_RMC) {
    nh->coarse = nh->p.data.rmc.coarse;
    nh->is_valid = nh->p.data.rmc.is_valid;
    nh->speed = nh->p.data.rmc.speed;
    nh->variation = nh->p.data.rmc.variation;
    nh->date = nh->p.data.rmc.date;
    nh->month = nh->p.data.rmc.month;
    nh->year = nh->p.data.rmc.year;
#endif /* NMEA_CFG_STATEMENT_GPRMC */
  }
  return 1;
}

/**
 * \brief           Init NMEA handle
 * \param[in]       gh: NMEA handle structure
 * \return          `1` on success, `0` otherwise
 */
uint8_t
nmea_init(nmea_t *nh) {
  memset(nh, 0x00, sizeof(*nh)); /* Reset structure */
  return 1;
}

/**
 * \brief           Process NMEA data from NMEA receiver
 * \param[in]       gh: NMEA handle structure
 * \param[in]       data: Received data
 * \param[in]       len: Number of bytes to process
 * \return          `1` on success, `0` otherwise
 */
uint8_t
nmea_process(nmea_t *nh, const void *data, size_t len) {
  const uint8_t *d = data;

  while (len--) { /* Process all bytes */
    if (*d == '$') { /* Check for beginning of NMEA line */
      memset(&nh->p, 0x00, sizeof(nh->p));/* Reset private memory */
      TERM_ADD(nh, *d); /* Add character to term */
    } else if (*d == ',') { /* Term separator character */
      parse_term(nh); /* Parse term we have currently in memory */
      CRC_ADD(nh, *d); /* Add character to CRC computation */
      TERM_NEXT(nh); /* Start with next term */
    } else if (*d == '*') { /* Start indicates end of data for CRC computation */
      parse_term(nh); /* Parse term we have currently in memory */
      nh->p.star = 1; /* STAR detected */
      TERM_NEXT(nh); /* Start with next term */
    } else if (*d == '\r') {
      if (check_crc(nh)) { /* Check for CRC result */
        /* CRC is OK, in theory we can copy data from statements to user data */
        copy_from_tmp_memory(nh); /* Copy memory from temporary to user memory */
      }
    } else {
      if (!nh->p.star) { /* Add to CRC only if star not yet detected */
        CRC_ADD(nh, *d); /* Add to CRC */
      }
      TERM_ADD(nh, *d); /* Add character to term */
    }
    d++; /* Process next character */
  }
  return 1;
}

/**
 * \brief           Calculate distance and bearing between `2` latitude and longitude coordinates
 * \param[in]       las: Latitude start coordinate, in units of degrees
 * \param[in]       los: Longitude start coordinate, in units of degrees
 * \param[in]       lae: Latitude end coordinate, in units of degrees
 * \param[in]       loe: Longitude end coordinate, in units of degrees
 * \param[out]      d: Pointer to output distance in units of meters
 * \param[out]      b: Pointer to output bearing between start and end coordinate in relation to north in units of degrees
 * \return          `1` on success, `0` otherwise
 */
uint8_t
nmea_distance_bearing(nmea_float_t las, nmea_float_t los, nmea_float_t lae, nmea_float_t loe, nmea_float_t *d, nmea_float_t *b) {
  nmea_float_t df, dfi, a;

  if (d == NULL && b == NULL) {
    return 0;
  }

  /* Convert degrees to radians */
  df = D2R(lae - las);
  dfi = D2R(loe - los);
  las = D2R(las);
  los = D2R(los);
  lae = D2R(lae);
  loe = D2R(loe);

  /*
   * Calculate distance
   *
   * Calculated distance is absolute value in meters between 2 points on earth.
   */
  if (d != NULL) {
    /*
     * a = sin(df / 2)^2 + cos(las) * cos(lae) * sin(dfi / 2)^2
     * *d = RADIUS * 2 * atan(a / (1 - a)) * 1000 (for meters)
     */
#if NMEA_CFG_DOUBLE
		a = FLT(sin(df * 0.5) * sin(df * 0.5) + sin(dfi * 0.5) * sin(dfi * 0.5) * cos(las) * cos(lae));
		*d = FLT(EARTH_RADIUS * 2.0 * atan2(sqrt(a), sqrt(1.0 - a)) * 1000.0);
#else /* NMEA_CFG_DOUBLE */
    a = FLT(sinf(df * 0.5f) * sinf(df * 0.5f) + sinf(dfi * 0.5f) * sinf(dfi * 0.5f) * cosf(las) * cosf(lae));
    *d = FLT(EARTH_RADIUS * 2.0f * atan2f(sqrtf(a), sqrtf(1.0f - a)) * 1000.0f);
#endif /* !NMEA_CFG_DOUBLE */
  }

  /*
   * Calculate bearing
   *
   * Bearing is calculated from point 1 to point 2.
   * Result will tell us in which direction (according to north) we should move,
   * to reach point 2.
   *
   * Example:
   *      Bearing is 0 => move to north
   *      Bearing is 90 => move to east
   *      Bearing is 180 => move to south
   *      Bearing is 270 => move to west
   */
  if (b != NULL) {
#if NMEA_CFG_DOUBLE
		df = FLT(sin(loe - los) * cos(lae));
		dfi = FLT(cos(las) * sin(lae) - sin(las) * cos(lae) * cos(loe - los));

		*b = R2D(atan2(df, dfi));               /* Calculate bearing and convert to degrees */
#else /* NMEA_CFG_DOUBLE */
    df = FLT(sinf(loe - los) * cosf(lae));
    dfi = FLT(cosf(las) * sinf(lae) - sinf(las) * cosf(lae) * cosf(loe - los));

    *b = R2D(atan2f(df, dfi)); /* Calculate bearing and convert to degrees */
#endif /* !NMEA_CFG_DOUBLE */
    if (*b < 0) { /* Check for negative angle */
      *b += FLT(360); /* Make bearing always positive */
    }
  }
  return 1;
}

/**
 * \brief           Convert NMEA speed (in knots = nautical mile per hour) to different speed format
 * \param[in]       sik: Speed in knots, received from NMEA statement
 * \param[in]       ts: Target speed to convert to from knots
 * \return          Speed calculated from knots
 */
nmea_float_t
nmea_to_speed(nmea_float_t sik, nmea_speed_t ts) {
  switch (ts) {
    case nmea_speed_kps:
      return FLT(sik * FLT(0.000514));
    case nmea_speed_kph:
      return FLT(sik * FLT(1.852));
    case nmea_speed_mps:
      return FLT(sik * FLT(0.5144));
    case nmea_speed_mpm:
      return FLT(sik * FLT(30.87));

    case nmea_speed_mips:
      return FLT(sik * FLT(0.0003197));
    case nmea_speed_mph:
      return FLT(sik * FLT(1.151));
    case nmea_speed_fps:
      return FLT(sik * FLT(1.688));
    case nmea_speed_fpm:
      return FLT(sik * FLT(101.3));

    case nmea_speed_mpk:
      return FLT(sik * FLT(32.4));
    case nmea_speed_spk:
      return FLT(sik * FLT(1944.0));
    case nmea_speed_sp100m:
      return FLT(sik * FLT(194.4));
    case nmea_speed_mipm:
      return FLT(sik * FLT(52.14));
    case nmea_speed_spm:
      return FLT(sik * FLT(3128.0));
    case nmea_speed_sp100y:
      return FLT(sik * FLT(177.7));

    case nmea_speed_smph:
      return FLT(sik * FLT(1.0));
    default:
      return 0;
  }
}

