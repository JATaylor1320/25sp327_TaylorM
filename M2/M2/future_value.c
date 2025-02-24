#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <limits.h>

long convert_to_long(char *strval) {
    char *endptr;
    errno = 0;
    long lval = strtol(strval, &endptr, 10);
    if (errno != 0 || *endptr != '\0') {
        return -1; // Indicate an error; caller will handle printing message.
    }
    return lval;
}

double convert_to_double(char *strval) {
    char *endptr;
    errno = 0;
    double dval = strtod(strval, &endptr);
    if (errno != 0 || *endptr != '\0') {
        return -1.0; // Indicate an error; caller will handle printing message.
    }
    return dval;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <monthly-investment> <annual-interest-rate> <number-of-years>\n", argv[0]);
        fprintf(stderr, "\n");
        fprintf(stderr, " monthly-investment: numeric value in range [1, 1000] representing the amount invested each month.\n");
        fprintf(stderr, "annual-interest-rate: decimal value in range [1, 15] representing the annual interest rate.\n");
        fprintf(stderr, "    number-of-years: integer value in range [1, 50] representing the duration of the investment in years.\n");
        return EXIT_FAILURE;
    }

    double monthly_investment;
    double annual_interest_rate;
    int years;

    monthly_investment = convert_to_double(argv[1]);
    if (errno != 0 || monthly_investment < 1 || monthly_investment > 1000 || monthly_investment == -1.0) {
        fprintf(stderr, "Error: Invalid input for monthly investment.\n");
        fprintf(stderr, "Usage: %s <monthly-investment> <annual-interest-rate> <number-of-years>\n", argv[0]);
        fprintf(stderr, "\n");
        fprintf(stderr, " monthly-investment: numeric value in range [1, 1000] representing the amount invested each month.\n");
        fprintf(stderr, "annual-interest-rate: decimal value in range [1, 15] representing the annual interest rate.\n");
        fprintf(stderr, "    number-of-years: integer value in range [1, 50] representing the duration of the investment in years.\n");
        return EXIT_FAILURE;
    }

    annual_interest_rate = convert_to_double(argv[2]);
    if (errno != 0 || annual_interest_rate < 1 || annual_interest_rate > 15 || annual_interest_rate == -1.0) {
        fprintf(stderr, "Error: Invalid input for annual interest rate.\n");
        fprintf(stderr, "Usage: %s <monthly-investment> <annual-interest-rate> <number-of-years>\n", argv[0]);
        fprintf(stderr, "\n");
        fprintf(stderr, " monthly-investment: numeric value in range [1, 1000] representing the amount invested each month.\n");
        fprintf(stderr, "annual-interest-rate: decimal value in range [1, 15] representing the annual interest rate.\n");
        fprintf(stderr, "    number-of-years: integer value in range [1, 50] representing the duration of the investment in years.\n");
        return EXIT_FAILURE;
    }

    years = convert_to_long(argv[3]);
    if (errno != 0 || years < 1 || years > 50 || years == -1) {
        fprintf(stderr, "Error: Invalid input for number of years.\n");
        fprintf(stderr, "Usage: %s <monthly-investment> <annual-interest-rate> <number-of-years>\n", argv[0]);
        fprintf(stderr, "\n");
        fprintf(stderr, " monthly-investment: numeric value in range [1, 1000] representing the amount invested each month.\n");
        fprintf(stderr, "annual-interest-rate: decimal value in range [1, 15] representing the annual interest rate.\n");
        fprintf(stderr, "    number-of-years: integer value in range [1, 50] representing the duration of the investment in years.\n");
        return EXIT_FAILURE;
    }

    printf("Monthly Investment: %.2f\n", monthly_investment);
    printf("Yearly Interest Rate: %.2f%%\n", annual_interest_rate);
    printf("Years: %d\n\n", years);

    double monthly_interest_rate = annual_interest_rate / 12.0 / 100.0;
    int months = years * 12;
    double future_value = 0;

    printf("Year Accum Interest      Value\n");
    printf("==== ============== ============\n");

    for (int year = 1; year <= years; ++year) {
        double yearly_interest = 0;
        double yearly_start_value = future_value;
        for (int month = 1; month <= 12; ++month) {
            future_value += monthly_investment;
            double monthly_interest_amount = future_value * monthly_interest_rate;
            future_value += monthly_interest_amount;
            if (month == 12) {
                yearly_interest = future_value - yearly_start_value - (monthly_investment * 12);
            }
        }
        printf("%4d %14.2f %12.2f\n", year, yearly_interest, future_value);
    }

    printf("\nFuture value after %d years is $%.2f\n", years, future_value);

    return EXIT_SUCCESS;
}


