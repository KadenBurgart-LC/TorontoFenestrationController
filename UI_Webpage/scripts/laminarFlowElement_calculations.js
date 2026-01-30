var LFE_calcs = {
	DiffPressure: 0.2875,      // in inWC
	AbsPressure: 14.932,       // in PSIA
	Temperature: 19.8,         // in °C
    WindowArea: 1.39,          // in m^2

    /* Get the humidity correction factor by taking table A-35600
     * from the Mariam LFE manual (NBSIR 83-2652), converting it into polynomials
     * for each humidity range, and then interpolating between polynomials for the
     * specified temperature and humidity.
     * tmp in °C, hum in %RH
     */
    GetHumidityCorrectionFactor: function(tmp, hum) {
        /* Helper to solve the cubic polynomials. 
         * Formula: (a * t^3) + (b * t^2) + (c * t) + d 
         */
        var solvePoly = function(t, a, b, c, d) {
            return (a * Math.pow(t, 3)) + (b * Math.pow(t, 2)) + (c * t) + d;
        };

        /* This is the humidity correction factor table that Delta uses.
         * We map the humidity levels to their specific polynomial coefficients.
         * Supposedly this comes from a Kestin and Whitelaw paper (1964) 
         * The Delta lab converted the table to work in °C.
         */
        var table = [
            { h: 0,   coeffs: [0,         0,          0,           1] },
            { h: 20,  coeffs: [-6.48e-8,  8.64e-7,   -5.7983e-5,   0.99955] },
            { h: 40,  coeffs: [-1.134e-7, 7.7914e-7, -0.0001, 0.9992] },
            { h: 60,  coeffs: [-1e-7, -1e-6, -0.0001, 0.9987] },
            { h: 80,  coeffs: [-1e-7, -4e-6, -0.0001, 0.9981] },
            { h: 100, coeffs: [-2e-7, -4e-6, -0.0002, 0.9977] }
        ];

        if (hum <= 0)   return 1;
        if (hum >= 100) return solvePoly(tmp, ...table[table.length - 1].coeffs);
        
        for (var i = 0; i < table.length - 1; i++) {
            var p0 = table[i];
            var p1 = table[i + 1];

            if (hum >= p0.h && hum <= p1.h) {
                // Solve the polynomial for the lower and upper humidity bounds at current temp
                var y0 = solvePoly(tmp, ...p0.coeffs);
                var y1 = solvePoly(tmp, ...p1.coeffs);

                // Linear interpolation between the two calculated correction factors
                // Formula: y = y0 + (x - x0) * (y1 - y0) / (x1 - x0)
                return y0 + (hum - p0.h) * (y1 - y0) / (p1.h - p0.h);
            }
        }
    },


    /* Get the relative humidity correction factor for air viscosity by taking
     * chart A-35500 from the Mariam LFE manual, based on the Kestin & Whitelaw data,
     * then converting that to polynomials for different humidity ranges, and 
     * interpolating between each range for the specified temperature and humidity.
     * tmp in °C, hum in %RH, units can be passed in as 'F' to support inferrior engineers.
     */
    GetViscosityCorrectionFactorForHumidity: function(tmp, hum, units = 'C'){
        /* Helper to solve the quadratic polynomials. 
         * Formula: (a * x^2) + (b * x) + c 
         */
        var solveQuad = function(x, a, b, c) {
            return (a * Math.pow(x, 2)) + (b * x) + c;
        };

        // The Mariam charts are in significantly inferrior units
        var tmpF = tmp;
        if (units == 'F') tmpF = tmp;
        else              tmpF = tmp * 1.8 + 32; 

        /* I took a screenshot of the chart, put it in excel, overlaid it with a scatter
         * graph, placed points along each line, and generated a best fit quadratic for each line.
         * The fits were visually perfect. These are the resulting polynomials...
         */
        var table = [
            { h: 0,  coeffs: [0,           0,         1] },
            { h: 10, coeffs: [-4.7568e-7,  6.4012e-5, 0.99788] },
            { h: 20, coeffs: [-1.2559e-6,  1.6423e-4, 0.99470] },
            { h: 30, coeffs: [-1.2467e-6, -1.4189e-4, 0.99606] },
            { h: 40, coeffs: [-1.6668e-6,  1.8175e-4, 0.99522] },
            { h: 50, coeffs: [-1.7138e-6,  1.7335e-4, 0.99585] },
            { h: 60, coeffs: [-1.8001e-6,  1.4973e-4, 0.99761] },
            { h: 90, coeffs: [-3.5227e-6,  3.2576e-4, 0.99239] }
        ];

        if (hum <= 0) return 1;
        if (hum > 90) hum = 90; // We don't have data over 90, but it should also be impossible to have this high of a humidity at the LFE

        for (var i = 0; i < table.length - 1; i++) {
            var p0 = table[i];
            var p1 = table[i + 1];

            if (hum >= p0.h && hum <= p1.h) {
                // Solve the polynomial for the lower and upper humidity bounds at current temp
                var y0 = solveQuad(tmpF, ...p0.coeffs);
                var y1 = solveQuad(tmpF, ...p1.coeffs);

                // Linear interpolation between the two calculated correction factors
                // Formula: y = y0 + (x - x0) * (y1 - y0) / (x1 - x0)
                return y0 + (hum - p0.h) * (y1 - y0) / (p1.h - p0.h);
            }
        }
    }
};

var FromScratchAttempt_NotWorkingRightNow = {
	/* Get the Pwet/Pdry humidity correction factor, 
	 * based on Table A-35600 (NBSIR 83-2652), called out by the user manual for the Mariam laminar flow element
	 */
	GetHumidityCorrectionFactor: function(tmp, hum){

	},

	/* Get the saturation vapor pressure of water at a given temperature, using the Buck equation.
	 * t in °C - returns pressure in kPa
	 */
	GetSatVaporPress_Buck: function(t) {
		return 0.61121 * Math.exp((18.678 - t / 234.5) * (t / (257.14 + t)));
	},

	/* Get the saturation vapor pressure of water at a given temperature, using the Magnus-Tetens equation.
	 * t in °C - returns pressure in kPa
	 */
	GetSatVaporPress_MagTet: function(t) {
		return 0.61078 * Math.exp((17.27 * t) / (t + 237.3));
	},

	WaterPartialPressure: function(t, rh) {
		var pSat = GetSatVaporPress_Buck(t);

		return (rh / 100) * pSat;
	},

	/* Calculate the mole fraction of water vapor
	 * t in °C, rh in %RH, p in kPa
	 */
	calcXw: function(t, rh, p){
	    var Pw = WaterPartialPressure(t, rh);

	    // Mole fraction cannot exceed 1.0
	    return Math.min(Pw / p);
	},

	/* Get the viscosity correction factor for wet air (μ wet/μ dry)
	 * based on A-35500 Kestin & Whitelaw, called out by the user manual for the Mariam laminar flow element
	 * tmp in °C, hum in %RH, p in kPa
	 */
	GetViscosityCorrectionFactor: function(tmp, hum, pres){

	    // Safety check: ensure pressure is provided and non-zero
	    if (!pres || pres <= 0) pres = 101.6;
		
		/* An equation based on the Studnikov modification of the Kestin-Whitelaw data */
		var getVCF = function(Mw, Ma, Xw, Xa, c) {
	        if (Xa === 0) return (Mw / Ma); // Prevent division by zero

			var num = 1 + (Mw / Ma) * (Xw / Xa) * c;
			var den = 1 + (Xw / Xa) * c

			return num / den;
		};

		var getVCF_2 = function(Mw, Ma, Xw, Xa) {
		    // Viscosity of pure components at ~100°F (approximate for the ratio)
		    // Mu_w: Water vapor viscosity, Mu_a: Dry air viscosity
		    // Note: The ratio Mu_a/Mu_w is roughly 1.5 to 1.6
		    var muW = 0.0102; 
		    var muA = 0.0185;

		    // Wilke interaction parameters (This is what Kestin & Whitelaw measured)
		    var phiAW = (Math.pow(1 + Math.sqrt(muA / muW) * Math.pow(Mw / Ma, 0.25), 2)) / 
		                (Math.sqrt(8) * Math.sqrt(1 + Ma / Mw));
		    
		    var phiWA = (Math.pow(1 + Math.sqrt(muW / muA) * Math.pow(Ma / Mw, 0.25), 2)) / 
		                (Math.sqrt(8) * Math.sqrt(1 + Mw / Ma));

		    // The actual mixture viscosity formula
		    var muWet = (Xa * muA / (Xa + Xw * phiAW)) + (Xw * muW / (Xw + Xa * phiWA));
		    
		    return muWet / muA;
		};

		var Mw = 18.015; // Molar mass of water vapor (g/mol)
		var Ma = 28.96;  // Molar mass of dry air (g/mol)
		var c = 2.75;    // An emperical constant based off the Kestin and Whitelaw data
		var Xw = calcXw(tmp, hum, pres); // Mole fraction of water vapor
		var Xa = 1 - Xw;	             // Mole fraction of dry air

	    return getVCF_2(Mw, Ma, Xw, Xa, c);
	},

	/* Stupid american units goddamnit */
	GetViscosityCorrectionFactor_F: function(tmp, hum, pres){
	    return GetViscosityCorrectionFactor((tmp-32)/1.8, hum, pres);
	},

	FtoC: function(f) { return (f-32)/1.8; }
};