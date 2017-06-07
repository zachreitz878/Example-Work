////////////////////////////////
//
// Reitz-Proj.c
// Zachary Reitz 105462168
// Semester Project CSCI 176 - Parallel Computing
// 5/6/2014
//
// OpenMP Pi Calculation--
//   Estimates the value of PI using the Gregory-Leibniz series
//
//   Runs properly whether parallelization is used or not.
//   However, the parallel version computes the sum in a different
//   order than the serial version. As the series gets longer, 
//   the numbers being added get incresingly smaller,
//   effecting the accuracy of the results.
//
//   Sequential is much less accurate than the Parallel function
//
////////////////////////////////

# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <omp.h>

//Function Prototypes
int main(int argc, char *argv[]);  	//Calls test function
void test(int log_n_max);		//Summates series to 10^(log_n_max-1)th term
double pi_seq(int n);			//Sequential pi calculation
double pi_omp(int n);			//Parallel pi calculation
double absolute(double x);		//Basic absolute value funciton

//MAIN function
int main(int argc, char *argv[])
{
  int log_n_max = 10; //determines how many test runs are executed
  double wtime;

  printf ( "\n\nCalculate PI using OpenMP with C" );
  printf ( "\n\nEstimate by summing a series." );
  printf ( "\n  # of processors = %d", omp_get_num_procs( ) );
  printf ( "\n  # of threads =    %d", omp_get_max_threads( ) );

  //start global timer
  wtime = omp_get_wtime( );

  test(log_n_max);

  //Calculate total execuation time
  wtime = omp_get_wtime( ) - wtime;

  printf ( "\nActual value of pi = 3.141592653589793" );
  printf ( "\nTotal execution time = %f seconds\n", wtime);

  return 0;
}

//Estimates PI 2 * (log_n_max -1) amount of times (SEQ and OMP in each run)
//Determines error, time, speedup, and efficiency of each series iteration
void test(int log_n_max)
{
  double error;
  double estimate;
  int log_n;
  char mode[4];
  int n;
  double pi = 3.141592653589793; //correct value of pi to 15 decimal places
  double wtime;
  double Ts, Tp, Sp, Ep; //seq exe time, omp exe time, speedup, efficiency

  printf ( "\n  N = number of terms computed and added" );
  printf ( "\n  Mode = SEQ for sequential, OMP for parallel" );
  printf ( "\n  Estimate = the computed estimate of PI" );
  printf ( "\n  Error = ( the computed estimate - PI )" );
  printf ( "\n  Sp(Speedup) = ( Ts / Tp )" );
  printf ( "\n  Ep(Efficiency) = Sp/p = ( Ts / p*Tp )\n" );
  printf ( "\n         N Mode      Estimate        Error         Time(s)     Sp        Ep\n\n" );

  n = 1;

  //Loops log_n_max - 1 times
  //Calculates Sequential Pi and OMP Pi in each iteration
  //Compares their error, time, speedup, and efficiency.
  for(log_n = 1; log_n <= log_n_max; log_n++)
  {
    //Sequential calculation enabled.
    strcpy( mode, "SEQ" );

    //start timer
    wtime = omp_get_wtime( );

    //Calculating Pi..
    estimate = pi_seq( n );

    //Calculate execution time..
    wtime = omp_get_wtime( ) - wtime;

    //Calculate error..
    error = absolute(estimate - pi);

    //Saving Sequential time for later
    Ts = wtime;

    //Print everything
    printf( "%10d  %s  %10.15f  %11g  %9f\n", n, mode, estimate, error, wtime );

    //Open MP calculation.
    strcpy( mode, "OMP" );

    //start timer
    wtime = omp_get_wtime( );
    
    //Calculating Pi..
    estimate = pi_omp( n );

    //Calculate execution time..
    wtime = omp_get_wtime( ) - wtime;

    //Calculate error..
    error = absolute(estimate - pi);

    //Calculating Speedup..
    Sp = Ts/wtime;

    //Calculating Efficiency..
    Ep = Sp / omp_get_max_threads( );

    //Print everything
    printf ( "%10d  %s  %10.15f  %11g  %9f  %5f  %5f\n", n, mode, estimate, error, wtime, Sp, Ep);

    //increment series limit 10 fold
    n = n * 10;
  }
  return;
}

/********PI CALCULATION**********
    The calculation is based on the formula for the indefinite integral:

      Integral 1 / ( 1 + X^2 ) dx = Arctan ( X ) 

    A standard way to approximate an integral uses the midpoint rule.
    If we create N equally spaced intervals of width 1/N, then the
    midpoint of the I-th interval is 

      X(i) = (2*i-1)/(2*N).  

    In the program we represent this by assigning variable h to
	h = 1/(2*n)
	and x becomes h*(2i-1)

    The approximation for the integral is then:

      Sum ( 1 <= I <= N ) (1/N) * 1 / ( 1 + X(i)^2 )

    In order to compute PI multiply this by 4
    We also can pull out the factor of 1/N, so that the formula you see 
    in the program looks like:

      ( 4 / N ) * Sum ( 1 <= I <= N ) 1 / ( 1 + X(i)^2 )
    
    So in our program
	sum = sum + 1/1(1+x*x)
	estimate = 4*sum/n

    Until roundoff becomes an issue, greater accuracy can be achieved by 
    increasing the value of N.
*********************************/
double pi_seq(int n)
{
  double h, estimate, sum, x;
  int i;

  h = 1.0/(double)(2*n);
  sum = 0.0;

  for (i = 1; i <= n; i++ )
  {
    x=h*(double)(2*i-1);
    sum += 1.0 / ( 1.0 + x * x );
  }
  estimate = 4.0 * sum / (double)(n);

  return estimate;
}

//Same as Sequential, but uses OpenMP to parralelize the series summation
double pi_omp(int n)
{
  double h, estimate, sum, x;
  int i;

  h = 1.0/(double)(2*n); //store h
  sum = 0.0;

//Parrallelize for loop
//Assigns n/num_threads amount of the loop to each thread
//Reduction on sum to lock critical section and signal
//threads to barrier before exiting the loop
//without reduciton, sum's value would be incorrect in estimate calculation
# pragma omp parallel \
  shared ( h, n ) \
  private ( i, x )

# pragma omp for reduction ( +: sum )

  for (i = 1; i <= n; i++ )
  {
    x=h*(double)(2*i-1);
    sum += 1.0 / ( 1.0 + x * x );
  }
  estimate = 4.0 * sum / (double)(n);

  return estimate;
}

//Returns the Absolute Value of a double variable
double absolute(double x)
{
  double value;

  if (0.0 <= x)
  {
    value = x;
  }
  else
  {
    value = - x;
  }
  return value;
}
