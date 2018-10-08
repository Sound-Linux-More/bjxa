/*-
 * Copyright (C) 2018  Dridi Boukelmoune
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

using System;
using System.IO;

namespace bjxa {
	class CliArgumentException: ArgumentException {
		public CliArgumentException(string message): base(message) { }
	}

	class Cli {
		static Stream xa;
		static Stream wav;

		static void Usage(TextWriter sw) {
			sw.WriteLine(String.Join(Environment.NewLine,
			    "Usage: bjxa.exe <action> [args...]",
			    "",
			    "Available actions:",
			    "",
			    "  help",
			    "    Show this message and exit.",
			    "",
			    "  decode [<xa file> [<wav file>]]",
			    "    Read an XA file and convert it into a " +
			        "WAV file.",
			    ""));
		}

		static void OpenFiles(string[] args) {
			if (args.Length > 1 && args[1] != "-")
				xa = File.Open(args[1], FileMode.Open);
			else
				xa = Console.OpenStandardInput();

			if (args.Length > 2 && args[2] != "-")
				wav = File.Create(args[2]);
			else
				wav = Console.OpenStandardOutput();
		}

		static void Decode() {
			Decoder dec = new Decoder();
			Console.Error.WriteLine("Decoder: " + dec);
			throw new InvalidOperationException("Unimplemented");
		}

		static void Exec(string[] args) {
			if (args.Length == 0)
				throw new
				    CliArgumentException("Missing an action");

			if (args[0] == "help") {
				Usage(Console.Out);
				return;
			}

			if (args[0] == "decode") {
				if (args.Length > 3)
					throw new CliArgumentException(
					    "Too many arguments");
				OpenFiles(args);
				Decode();
				return;
			}

			throw new CliArgumentException("Unknown action");
		}

		static int Main(string[] args) {
			try {
				Exec(args);
				return (0);
			}
			catch (Exception e) {
				Console.Error.Write("bjxa: ");
				Console.Error.WriteLine(e.Message);
				if (e is CliArgumentException)
					Usage(Console.Error);
				return (1);
			}
			finally {
				if (xa != null)
					xa.Dispose();
				if (wav != null)
					wav.Dispose();
			}
		}
	}
}
