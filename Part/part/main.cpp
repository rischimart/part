#include <Netlist.hpp>
#include <FMException.hpp>
#include <FMParam.hpp>
#include <MlPartTmpl.hpp>
#include <FMPartTmpl.hpp>
#include <FMPartTmpl4.hpp>
#include <FMBiPartCore.hpp>
#include <FMBiPartCore4.hpp>
#include <FMBiGainMgr.hpp>
#include <FMBiGainMgr2.hpp>
#include <FMKWayPartCore.hpp>
#include <FMKWayPartCore4.hpp>
#include <FMKWayGainMgr2.hpp>
#include <Netlist.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <climits>  //added by Zhou


using namespace std;

/* Partition mode */
enum mode
  {
    MODE_COMPLETE,               /**< Normal run */
    MODE_INCREMENTAL,            /**< Increamental mode */
    MODE_CNT                     /**< Number of modes */
  };

enum input_fmt
  {
    FORMAT_NETD,
    FORMAT_HGR,
    FORMAT_XML,
    FORMAT_CNT
  };

/** Cost model option */
enum cost_model
  {
    COST_MODEL_HEDGE,            /**< hyper edge cost */
    COST_MODEL_KMINUS1,          /**< K-1 cost */
    COST_MODEL_SOED,             /**< Sum-Of-External-Degrees */
    COST_MODEL_TSV               // TSV(3D IC) cost
  };

/* A single command-line option. */
struct option
  {
    const char *long_name;        /* Long name (|"--name"|). */
    int short_name;               /* Short name (|"-n"|); value returned. */
    int has_arg;                  /* Has a required argument? */
  };


/** Program options. */
struct test_options
  {
    enum mode mode;               /**< Complete or increamental mode */
    enum input_fmt input_fmt;     /**< Input format */
    enum cost_model cost_model;   /**< Cost Model */
    const char* file_name;        /**< Input file name */
    const char* initsol_name;     /**< Initial partition file name */
    int num_parts;                /**< Number of partitions */
    double balTol;                /**< Balance tolerance */
    int iter_cnt;                 /**< Number of runs. */
    int seed_given;               /**< Seed provided on command line? */
    unsigned int seed;            /**< Random number seed. */
    int verbosity;                /**< Verbosity level, 0=default. */
  };


/* Program name. */
const char *pgm_name;


/* Prints |message| on |stderr|, which is formatted as for |printf()|,
   and terminates the program unsuccessfully. */
static void
fail (const char *message, ...)
{
  va_list args;

  fprintf (stderr, "%s: ", pgm_name);

  va_start (args, message);
  vfprintf (stderr, message, args);
  va_end (args);

  putchar ('\n');

  exit (EXIT_FAILURE);
}

/* Option parsing state. */
struct option_state
  {
    const struct option *options; /* List of options. */
    const char **arg_next;        /* Remaining arguments. */
    const char *short_next;       /* When non-null, unparsed short options. */
  };

/* Creates and returns a command-line options parser.
   |args| is a null-terminated array of command-line arguments, not
   including program name. */
static struct option_state *
option_init (const struct option *options, const char **args)
{
  struct option_state *state;

  assert (options != NULL && args != NULL);

  state = new option_state;
  state->options = options;
  state->arg_next = args;
  state->short_next = NULL;

  return state;
}

/* Parses a short option whose single-character name is pointed to by
   |state->short_next|.  Advances past the option so that the next one
   will be parsed in the next call to |option_get()|.  Sets |*argp| to
   the option's argument, if any.  Returns the option's short name. */
static int
handle_short_option (struct option_state *state, const char **argp)
{
  const struct option *o;

  assert (state != NULL
          && state->short_next != NULL && *state->short_next != '\0'
          && state->options != NULL);

  /* Find option in |o|. */
  for (o = state->options; ; o++)
    if (o->long_name == NULL)
      fail ("unknown option `-%c'; use --help for help", *state->short_next);
    else if (o->short_name == *state->short_next)
      break;
  state->short_next++;

  /* Handle argument. */
  if (o->has_arg)
    {
      if (*state->arg_next == NULL || **state->arg_next == '-')
        fail ("`-%c' requires an argument; use --help for help");

      *argp = *state->arg_next++;
    }

  return o->short_name;
}

/* Parses a long option whose command-line argument is pointed to by
   |*state->arg_next|.  Advances past the option so that the next one
   will be parsed in the next call to |option_get()|.  Sets |*argp| to
   the option's argument, if any.  Returns the option's identifier. */
static int
handle_long_option (struct option_state *state, const char **argp)
{
  const struct option *o;        /* Iterator on options. */
  char name[16];                 /* Option name. */
  const char *arg;               /* Option argument. */

  assert (state != NULL
          && state->arg_next != NULL && *state->arg_next != NULL
          && state->options != NULL
          && argp != NULL);

  /* Copy the option name into |name|
     and put a pointer to its argument, or |NULL| if none, into |arg|. */
  {
    const char *p = *state->arg_next + 2;
    const char *q = p + strcspn (p, "=");
    size_t name_len = q - p;

    if (name_len > (sizeof name) - 1)
      name_len = (sizeof name) - 1;
    memcpy (name, p, name_len);
    name[name_len] = '\0';

    arg = (*q == '=') ? q + 1 : NULL;
  }

  /* Find option in |o|. */
  for (o = state->options; ; o++)
    if (o->long_name == NULL)
      fail ("unknown option --%s; use --help for help", name);
    else if (!strcmp (name, o->long_name))
      break;

  /* Make sure option has an argument if it should. */
  if ((arg != NULL) != (o->has_arg != 0))
    {
      if (arg != NULL)
        fail ("--%s can't take an argument; use --help for help", name);
      else
        fail ("--%s requires an argument; use --help for help", name);
    }

  /* Advance and return. */
  state->arg_next++;
  *argp = (char *) arg;
  return o->short_name;
}

/* Retrieves the next option in the state vector |state|.
   Returns the option's identifier, or -1 if out of options.
   Stores the option's argument, or |NULL| if none, into |*argp|. */
static int
option_get (struct option_state *state, const char **argp)
{
  assert (state != NULL && argp != NULL);

  /* No argument by default. */
  *argp = NULL;

  /* Deal with left-over short options. */
  if (state->short_next != NULL)
    {
      if (*state->short_next != '\0')
        return handle_short_option (state, argp);
      else
        state->short_next = NULL;
    }

  /* Out of options? */
  if (*state->arg_next == NULL)
    {
      free (state);
      return -1;
    }

  /* Non-option arguments not supported. */
  if ((*state->arg_next)[0] != '-')
    {
      //xxx fail ("non-option arguments encountered; use --help for help");
      *argp = *state->arg_next;
      return -2;
    }

  if ((*state->arg_next)[1] == '\0')
    fail ("unknown option `-'; use --help for help");

  /* Handle the option. */
  if ((*state->arg_next)[1] == '-')
    return handle_long_option (state, argp);
  else
    {
      state->short_next = *state->arg_next + 1;
      state->arg_next++;
      return handle_short_option (state, argp);
    }
}



/* Command line parser. */

/* If |a| is a prefix for |b| or vice versa, returns the length of the
   match.
   Otherwise, returns 0. */
size_t
match_len (const char *a, const char *b)
{
  size_t cnt;

  for (cnt = 0; *a == *b && *a != '\0'; a++, b++)
    cnt++;

  return (*a != *b && *a != '\0' && *b != '\0') ? 0 : cnt;
}

/* |s| should point to a decimal representation of an integer.
   Returns the value of |s|, if successful, or 0 on failure. */
static int
stoi (const char *s)
{
  long x = strtol (s, NULL, 10);
  return x >= INT_MIN && x <= INT_MAX ? x : 0;
}


/* Print helpful syntax message and exit. */
static void
usage (void)
{
  static const char *help[] =
    {
      "Usage: %s [OPTION]... [FILE]...\n",
      "Perform netlist partitioning.\n\n",
      "Mandatory arguments to long options are mandatory for short options too.\n",
//xxx "-m, --mode=MODE     Sets mode to perform.  MODE is one of:\n",
//xxx "                      complete    normal partitioning (default)\n",
//xxx "                      incremental start from known initial solution.\n",
      "-k, --nparts=NUM    Sets number of partitions to NUM (default is 2).\n",
      "-f, --format=FORMAT Sets input file format.  FORMAT is one of:\n",
      "                      netD        IBM .net/.netD format (default)\n",
      "                      hgr         hMetis .hgr format\n",
      "-c, --cost          Sets the cost model to be minimize\n",
      "                      HEdge       hyperedge cost (default)\n",
      "                      KMinus1     (K-1) cost\n",
      "                      SOED         Sum-Of-External-Degrees cost\n"
      "                      TSV         number of TSVs for 3D IC partitioning \n"   //added by Zhou
      "-i, --initsol=name  Uses given hmetis partition file as initial\n",
      "-b, --balance=TOL   Sets the balance tolerant to TOL in percentage.\n",
      "                    (default is 2.0)\n",
      "-r, --runs=RUNS     Sets number of runs to RUNS (default is 3).\n",
      "-S, --seed=SEED     Sets initial number seed to SEED.\n",
      "                    (default is 1)\n",
      "-q, --quiet         Turns down verbosity level.\n",
      "-v, --verbose       Turns up verbosity level.\n",
      "-h, --help          Displays this help screen.\n",
      "-V, --version       Reports version and copyright information.\n\n",
      "Report bugs to <waishing_luk@ieee.org>\n",
      NULL,
    };

  const char **p;
  for (p = help; *p != NULL; p++)
    printf (*p, pgm_name);

  exit (EXIT_SUCCESS);
}

/* Parses command-line arguments from null-terminated array |args|.
   Sets up |options| appropriately to correspond. */
static void
parse_command_line (const char **args, struct test_options *options)
{
  static const struct option option_tab[] =
    {
      {"mode",    'm', 1},
      {"nparts",  'k', 1},
      {"format",  'f', 1},
      {"cost",    'c', 1},
      {"initsol", 'i', 1},
      {"balance", 'b', 1},
      {"runs",    'r', 1},
      {"seed",    'S', 1},
      {"quiet",   'q', 0},
      {"verbose", 'v', 0},
      {"help",    'h', 0},
      {"version", 'V', 0},
      {NULL, 0, 0},
    };

  struct option_state *state;

  /* Default options. */
  options->mode = MODE_COMPLETE;
  options->input_fmt = FORMAT_NETD;
  options->cost_model = COST_MODEL_HEDGE;
  options->initsol_name = 0;
  options->num_parts = 2;                 // 2 partitions
  options->balTol = 2.0;                  // 2%
  options->iter_cnt = 3;                  // 3 runs
  options->seed_given = 0;
  options->verbosity = 0;

  if (*args == NULL)
    return;

  state = option_init (option_tab, args + 1);
  for (;;)
    {
      const char *arg;
      int id = option_get (state, &arg);
      if (id == -1)
        break;
      if (id == -2) {
        options->file_name = arg;
        break;
      }
      switch (id)
        {
        case 't':
          if (match_len (arg, "complete") >= 3)
            options->mode = MODE_COMPLETE;
          else if (match_len (arg, "incremental") >= 3)
            options->mode = MODE_INCREMENTAL;
          else
            fail ("unknown mode \"%s\"", arg);
          break;

        case 'f':
          if (match_len (arg, "netD") >= 3)
            options->input_fmt = FORMAT_NETD;
          else if (match_len (arg, "hgr") >= 3)
            options->input_fmt = FORMAT_HGR;
          else if (match_len (arg, "xml") >= 3)
            options->input_fmt = FORMAT_XML; // not implement yet
          else
            fail ("unknown input format \"%s\"", arg);
          break;

        case 'c':
          if (match_len (arg, "HEdge") >= 3)
            options->cost_model = COST_MODEL_HEDGE;
          else if (match_len (arg, "KMinus1") >= 3)
            options->cost_model = COST_MODEL_KMINUS1;
          else if (match_len (arg, "SOED") >= 3)
            options->cost_model = COST_MODEL_SOED;
          else if (match_len (arg, "TSV") >= 3)
            options->cost_model = COST_MODEL_TSV;
          else
            fail ("unknown input format \"%s\"", arg);
          break;

        case 'i':
          options->initsol_name = arg;
          break;

        case 'b':
          options->balTol = atof (arg);
          if (options->balTol <= 0.0 || options->balTol >= 100.0)
            fail ("bad balance tolerant \"%s\"", arg);
          break;

        case 'k':
          options->num_parts = stoi (arg);
          if (options->num_parts < 2)
            fail ("bad number of partitions \"%s\"", arg);
          break;

        case 'r':
          options->iter_cnt = stoi (arg);
          if (options->iter_cnt < 1)
            fail ("bad number of runs \"%s\"", arg);
          break;

        case 'S':
          options->seed_given = 1;
          options->seed = strtoul (arg, NULL, 0);
          break;

        case 'q':
          options->verbosity--;
          break;

        case 'v':
          options->verbosity++;
          break;

        case 'h':
          usage ();
          break;

        case 'V':
          fputs ("Netlist Partitioner\n"
                 "Copyright (C) 1998-2003 Wai-Shing Luk.\n"
                 "This program comes with NO WARRANTY, not even for\n"
                 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
                 "You may redistribute copies under the terms of the\n"
                 "GNU General Public License.  For more information on\n"
                 "these matters, see the file named COPYING.\n",
                 stdout);
          exit (EXIT_SUCCESS);

        default:
          assert (0);
        }
    }
}

/* Choose and return an initial random seed based on the current time.
   Based on code by Lawrence Kirby <fred@genesis.demon.co.uk>. */
unsigned
time_seed (void)
{
  time_t timeval;        /* Current time. */
  unsigned char *ptr;        /* Type punned pointed into timeval. */
  unsigned seed;        /* Generated seed. */
  size_t i;

  timeval = time (NULL);
  ptr = (unsigned char *) &timeval;

  seed = 0;
  for (i = 0; i < sizeof timeval; i++)
    seed = seed * (UCHAR_MAX + 2u) + ptr[i];

  return seed;
}

int main(int, const char* argv[])
{
  struct test_options opts;        /* Command-line options. */
  int success;                     /* Everything okay so far? */

  /* Initialize |pgm_name|, using |argv[0]| if sensible. */
  pgm_name = argv[0] != NULL && argv[0][0] != '\0' ? argv[0] : "nl-part";

  /* Parse command line into |options|. */
  parse_command_line (argv, &opts);

  if (opts.verbosity >= 0)
    fputs ("Netlist partitioner 2.0.1; use --help to get help.\n", stdout);

  if (!opts.seed_given)
    opts.seed = time_seed () % 32768u;

  /* Run the tests. */
  success = 1;

  Netlist H;

  string netDFile = string(opts.file_name) + ".netD";
  if (!H.readNetD(netDFile.c_str())) {
    string netFile = string(opts.file_name) + ".net";
    if (!H.readNetD(netFile.c_str())) {
      fail ("Error: could not read \"%s.netD\"", opts.file_name);
    }
  }

  string areFile = string(opts.file_name) + ".are";
  ifstream in(areFile.c_str());
  if (in.fail()) {
    cout << "Warning: could not read " << opts.file_name << ".are" << endl;
  }
  else {
    H.readAre(areFile.c_str());
  }

  if (opts.verbosity >= 0) {
    cout << "#cells = "   << H.getNumCells()  << ", ";
    cout << "#nets = "    << H.getNumNets()   << ", ";
    cout << "#pins = "    << H.getNumPins()   << ", ";
    cout << "#max_deg = " << H.getMaxDegree() << endl;
  }

  std::vector<unsigned char> part(H.getNumCells());
  if (opts.initsol_name != 0) {
    if (!H.readHMetisSol(opts.initsol_name, part)) {
      fail ("could not read \"%s\"", opts.initsol_name);
    }
  }

  srand (opts.seed);

  try {
    if (opts.num_parts == 2) {
      typedef
      MlPartTmpl
      < FMBiPartMgrBase,
        FMPartTmpl<FMBiPartCore, FMBiGainMgr2>,
        FMPartTmpl4<FMBiPartCore4, FMBiGainMgr2>
      > MlBiPartMgr;

      const FMParam param(H,2,0);
      MlBiPartMgr P(param);

      if (opts.initsol_name != 0) {
        if (!P.checkBalanceOk(part)) {
          cout << "Warning: illegal initial solution." << endl;
        }
        P.setNoInit(P.cutCost(part));
      }
      P.setVerbosity(opts.verbosity);
      P.setBalanceTol(opts.balTol/100.0);

      // P.setContractMethod(&Netlist::contractByMMC);
      double elapsed = P.doPartition(part, opts.iter_cnt);

      if (opts.verbosity >= 0) {
        cout << "cut-cost: " << P.cutCost(part)
             << ", diff: "   << P.diff() << endl;
        cout << "Elapsed partitioning time : " << elapsed << endl;
      }
    }
    else {
      typedef
      MlPartTmpl
      < FMKWayPartMgrBase,
        FMPartTmpl<FMKWayPartCore, FMKWayGainMgr2>,
        FMPartTmpl4<FMKWayPartCore4, FMKWayGainMgr2>
      > MlKWayPartMgr;

      const FMParam param(H, opts.num_parts, opts.cost_model);
      MlKWayPartMgr P(param);
      P.setBalanceTol(opts.balTol/100.0);
      if (opts.initsol_name != 0) {
        if (!P.checkBalanceOk(part)) {
          cout << "Warning: illegal initial solution." << ", ";
        }
        const unsigned int cost = P.cutCost(part);
        P.setNoInit(cost);
        cout << "InitCost: " << cost << endl;
      }

      // P.setContractMethod(&Netlist::contractByMMC);
      double elapsed = P.doPartition(part, opts.iter_cnt);

      if (opts.verbosity >= 0) {
        cout << "cut-cost: " << P.cutCost(part)
             << ", diff: "   << P.diff() << endl;
        cout << "Elapsed partitioning time : " << elapsed << endl;
      }
    }

    string solFile = string(opts.file_name) + ".sol";
    if (!H.writeSol(solFile.c_str(), opts.num_parts, part)) {
      fail ("could not write \"%s.sol\"", opts.file_name);
    }

  } catch (FMException& e) {
    fail ("error: %s", e.what());
  }

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
