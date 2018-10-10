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
using System.Reflection;

[assembly:AssemblyVersionAttribute("0.0")]

namespace bjxa {
	public class Format {
		public const int HEADER_SIZE_XA = 32;
		public const int HEADER_SIZE_RIFF = 44;

		public uint	DataLengthPcm;
		public uint	Blocks;
		public byte	BlockSizePcm;
		public byte	BlockSizeXa;
		public ushort	SamplesRate;
		public byte	SampleBits;
		public byte	Channels;

		public byte[] WritePcm(byte[] dst, short[] pcm) {
			throw new NotImplementedException("WritePcm");
		}

		public int WritePcm(Stream wav, short[] pcm) {
			throw new NotImplementedException("WritePcm");
		}
	}

	public class Decoder {
		public Format ReadHeader(byte[] xa) {
			throw new NotImplementedException("ReadHeader");
		}

		public Format ReadHeader(Stream xa) {
			throw new NotImplementedException("ReadHeader");
		}

		public byte[] WriteRiffHeader() {
			throw new NotImplementedException("WriteRiffHeader");
		}

		public int WriteRiffHeader(Stream wav) {
			throw new NotImplementedException("WriteRiffHeader");
		}

		public int Decode(byte[] xa, short[] pcm) {
			throw new NotImplementedException("Decode");
		}
	}
}
