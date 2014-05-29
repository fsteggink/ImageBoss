// RasterImage_JPEG.cpp
// FS, 10-03-2007

#define DLLAPI_RASTERIMAGE_JPEG __declspec(dllexport)

#include "RasterImage_JPEG.h"

boost::shared_ptr<LoadPlugin_JPEG> g_LoadPlugin;
boost::shared_ptr<SavePlugin_JPEG> g_SavePlugin;
boost::shared_ptr<InfoPlugin_JPEG> g_InfoPlugin;
boost::shared_ptr<LoadByLinePlugin_JPEG> g_LoadByLinePlugin;
boost::shared_ptr<SaveByLinePlugin_JPEG> g_SaveByLinePlugin;

const int JPEG_SAVE_QUALITY = 85;


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

    return TRUE;
}


bool DLLAPI_RASTERIMAGE_JPEG SupportsInterface(Interface eIF)
{
	return (g_ceSupportedInterfaces & eIF) != 0;
}


Interface DLLAPI_RASTERIMAGE_JPEG SupportedInterfaces()
{
	return g_ceSupportedInterfaces;
}


boost::shared_ptr<IImagePlugin> DLLAPI_RASTERIMAGE_JPEG CreatePlugin(Interface eIF)
{
	switch(eIF)
	{
	case Interface::RasterLoad:
		g_LoadPlugin = boost::shared_ptr<LoadPlugin_JPEG>(new LoadPlugin_JPEG());
		return g_LoadPlugin;
		break;

	case Interface::RasterSave:
		g_SavePlugin = boost::shared_ptr<SavePlugin_JPEG>(new SavePlugin_JPEG());
		return g_SavePlugin;
		break;

	case Interface::RasterInfo:
		g_InfoPlugin = boost::shared_ptr<InfoPlugin_JPEG>(new InfoPlugin_JPEG());
		return g_InfoPlugin;
		break;

	case Interface::RasterLineLoad:
		g_LoadByLinePlugin = boost::shared_ptr<LoadByLinePlugin_JPEG>(new LoadByLinePlugin_JPEG());
		return g_LoadByLinePlugin;
		break;

	case Interface::RasterLineSave:
		g_SaveByLinePlugin = boost::shared_ptr<SaveByLinePlugin_JPEG>(new SaveByLinePlugin_JPEG());
		return g_SaveByLinePlugin;
		break;
	}

	return boost::shared_ptr<IImagePlugin>();
}


FileMode LoadPlugin_JPEG::CanLoad(const MimeType &mimeType) const
{
	if(mimeType == MIME_JPEG)
		return FileMode::All;
	else
		return FileMode::None;
}


boost::shared_ptr<IRasterLoadByLinePlugin> LoadPlugin_JPEG::GetLoadByLinePlugin() const
{
	return boost::shared_ptr<IRasterLoadByLinePlugin>(new LoadByLinePlugin_JPEG());
}


FileMode SavePlugin_JPEG::CanSave(const MimeType &mimeType) const
{
	if(mimeType == MIME_JPEG)
		return FileMode::All;
	else
		return FileMode::None;
}


boost::shared_ptr<IRasterSaveByLinePlugin> SavePlugin_JPEG::GetSaveByLinePlugin() const
{
	return boost::shared_ptr<IRasterSaveByLinePlugin>(new SaveByLinePlugin_JPEG());
}


bool InfoPlugin_JPEG::CanInfo(const MimeType &mimeType) const
{
	return (mimeType == MIME_JPEG);
}


void InfoPlugin_JPEG::Info(const std::string &fileName, int &width, int &height,
						   int &samplesPerPixel, int &bitsPerSample) const
{
	width = height = samplesPerPixel = bitsPerSample = -1;

	LoadByLinePlugin_JPEG lblp;
	IContext *ctxt = lblp.Open(fileName);

	width = lblp.GetWidth(ctxt);
	height = lblp.GetHeight(ctxt);
	samplesPerPixel = lblp.GetSamplesPerPixel(ctxt);
	bitsPerSample = 8;

	lblp.Close(ctxt);
}


FileMode LoadByLinePlugin_JPEG::CanLoad(const MimeType &mimeType) const
{
	if(mimeType == MIME_JPEG)
		return FileMode::All;
	else
		return FileMode::None;
}


IContext *LoadByLinePlugin_JPEG::Open(const std::string &fileName) const
{
	m_fIn.open(fileName.c_str(), std::ios::binary);
	if(!m_fIn)
		throw ImageBossException(EXC_CANT_LOAD_FILE);

	IContext *context = Open(m_fIn);
	dynamic_cast<LoadByLineContext *>(context)->isFile = true;

	return context;
}


IContext *LoadByLinePlugin_JPEG::Open(std::istream &input) const
{
	if(!input)
		throw ImageBossException(EXC_INVALID_INPUTSTREAM);

	LoadByLineContext *oContext = new LoadByLineContext(input);

	try
	{
		oContext->cinfo.err = jpeg_std_error(&oContext->jerr);
		oContext->cinfo.err->error_exit = 0;
		
		// Read file header
		input.seekg(0, std::ios::end);

		jpeg_create_decompress(&oContext->cinfo);
		imageboss_jpeg_stdio_src(&oContext->cinfo, &input);
		jpeg_read_header(&oContext->cinfo, TRUE);
		jpeg_start_decompress(&oContext->cinfo);
	}
	catch(...)
	{
		// Ignore
	}

	return oContext;
}


void LoadByLinePlugin_JPEG::ReadScanline(IContext *context, RasterLayer<byte>::Scanline &scanline) const
{
	if(!context)
		return;

	LoadByLineContext *oContext = dynamic_cast<LoadByLineContext *>(context);
	JSAMPROW sampRow = const_cast<byte *>(&scanline[0]);
	jpeg_read_scanlines(&oContext->cinfo, &sampRow, 1);

	oContext->startedDecompress = true;

	return;
}


void LoadByLinePlugin_JPEG::Close(IContext *&context) const
{
	if(!context)
		return;

	LoadByLineContext *oContext = dynamic_cast<LoadByLineContext *>(context);
	if(oContext->startedDecompress)
		jpeg_finish_decompress(&oContext->cinfo);
	else
		jpeg_abort_decompress(&oContext->cinfo);
	jpeg_destroy_decompress(&oContext->cinfo);

	delete context;
	context = 0;

	if(m_fIn)
		m_fIn.close();

	return;
}


int LoadByLinePlugin_JPEG::GetWidth(IContext *context) const
{
	return dynamic_cast<LoadByLineContext *>(context)->cinfo.image_width;
}


int LoadByLinePlugin_JPEG::GetHeight(IContext *context) const
{
	return dynamic_cast<LoadByLineContext *>(context)->cinfo.image_height;
}


int LoadByLinePlugin_JPEG::GetSamplesPerPixel(IContext *context) const
{
	return dynamic_cast<LoadByLineContext *>(context)->cinfo.output_components;
}


bool LoadByLinePlugin_JPEG::HasPalette(IContext *context) const
{
	// JPEG files have no palette
	return false;
}


const RasterLayer<byte>::Palette &LoadByLinePlugin_JPEG::GetPalette(IContext *context) const
{
	// JPEG files have no palette
	throw EXC_METHOD_NOT_IMPLEMENTED;
}


FileMode SaveByLinePlugin_JPEG::CanSave(const MimeType &mimeType) const
{
	if(mimeType == MIME_JPEG)
		return FileMode::All;
	else
		return FileMode::None;
}


IContext *SaveByLinePlugin_JPEG::Open(const std::string &fileName, int width, int height, int spp) const
{
	m_fOut.open(fileName.c_str(), std::ios::binary);
	if(!m_fOut)
		throw ImageBossException(EXC_CANT_SAVE_FILE);

	// NOTE: Open(filename) does not work well! Probably because context is copied, and that in
	// combination with a reference to the stream, might cause problems.
	IContext *context = Open(m_fOut, width, height, spp);
	dynamic_cast<SaveByLineContext *>(context)->isFile = true;

	return context;
}


IContext *SaveByLinePlugin_JPEG::Open(std::ostream &output, int width, int height, int spp) const
{
	if(!output)
		throw ImageBossException(EXC_INVALID_OUTPUTSTREAM);
	if(width <= 0 || height <= 0)
		throw ImageBossException(EXC_INVALID_DIMENSIONS);
	if(width > MAX_JPEG_SIZE || height > MAX_JPEG_SIZE)
		throw ImageBossException(EXC_INVALID_DIMENSIONS);
	if(spp != 3)
		throw ImageBossException(EXC_INVALID_SAMPLE_COUNT);

	SaveByLineContext *oContext = new SaveByLineContext(output);

	try
	{
		oContext->cinfo.err = jpeg_std_error(&oContext->jerr);
		oContext->cinfo.err->error_exit = 0;
		
		jpeg_create_compress(&oContext->cinfo);
		imageboss_jpeg_stdio_dest(&oContext->cinfo, &output);

		oContext->cinfo.image_width = width;
		oContext->cinfo.image_height = height;
		oContext->cinfo.input_components = 3;
		oContext->cinfo.in_color_space = JCS_RGB;

		jpeg_set_defaults(&oContext->cinfo);	
		jpeg_set_quality(&oContext->cinfo, JPEG_SAVE_QUALITY, false);
		
		jpeg_start_compress(&oContext->cinfo, TRUE);
	}
	catch(...)
	{
		// Ignore
	}


	return oContext;
}


void SaveByLinePlugin_JPEG::WriteScanline(IContext *context, const RasterLayer<byte>::Scanline &scanline) const
{
	if(!context)
		return;

	SaveByLineContext *oContext = dynamic_cast<SaveByLineContext *>(context);
	char *arrSL = reinterpret_cast<char *>(const_cast<byte *>(&scanline[0]));
	jpeg_write_scanlines(&oContext->cinfo, reinterpret_cast<JSAMPARRAY>(&arrSL), 1);

	return;
}


void SaveByLinePlugin_JPEG::Close(IContext *&context) const
{
	if(!context)
		return;

	SaveByLineContext *oContext = dynamic_cast<SaveByLineContext *>(context);
	jpeg_finish_compress(&oContext->cinfo);
	jpeg_destroy_compress(&oContext->cinfo);

	delete context;
	context = 0;

	if(m_fOut)
		m_fOut.close();
}



///////////////////////////////////////////////////////////////////////////////
// Below code describes the source and destination managers for compressed JPEG
// data. I've taken the source and destination managers for stdio streams 
// (files), and modified them to handle bytestreams. Origin is the source code
// of Libjpeg, files jdatadst.c and jdatasrc.c. Strange mix of C and C++ ;)
///////////////////////////////////////////////////////////////////////////////

#define OUTPUT_BUF_SIZE	8192	/* choose an efficiently fwrite'able size */
#define INPUT_BUF_SIZE	8192	/* choose an efficiently fread'able size */
#define SIZEOF(object)	((size_t) sizeof(object))

//#define JFREAD(file,buf,sizeofbuf)  \
//  ((size_t) fread((void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))
#define JFREAD(stream,buf,sizeofbuf)  \
	(stream)->read((char *)(buf), (sizeofbuf))

//#define JFWRITE(file,buf,sizeofbuf)  \
//  ((size_t) fwrite((const void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))
#define JFWRITE(stream,buf,sizeofbuf)  \
	(stream)->write((char *)(buf), (sizeofbuf))



/* Expanded data destination object for stdio output */

typedef struct 
{
	struct jpeg_destination_mgr pub;	/* public fields */

	std::ostream *outstream;					/* target stream */
	JOCTET * buffer;					/* start of buffer */
} my_destination_mgr;

typedef my_destination_mgr * my_dest_ptr;

/*
 * Initialize destination --- called by jpeg_start_compress
 * before any data is actually written.
 */

METHODDEF(void)
init_destination (j_compress_ptr cinfo)
{
	my_dest_ptr dest = (my_dest_ptr) cinfo->dest;

	/* Allocate the output buffer --- it will be released when done with image */
	dest->buffer = (JOCTET *)
		(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
		OUTPUT_BUF_SIZE * SIZEOF(JOCTET));

	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

	dest->outstream->clear();
}

/*
 * Empty the output buffer --- called whenever buffer fills up.
 *
 * In typical applications, this should write the entire output buffer
 * (ignoring the current state of next_output_byte & free_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been dumped.
 *
 * In applications that need to be able to suspend compression due to output
 * overrun, a FALSE return indicates that the buffer cannot be emptied now.
 * In this situation, the compressor will return to its caller (possibly with
 * an indication that it has not accepted all the supplied scanlines).  The
 * application should resume compression after it has made more room in the
 * output buffer.  Note that there are substantial restrictions on the use of
 * suspension --- see the documentation.
 *
 * When suspending, the compressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_output_byte & free_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point will be regenerated after resumption, so do not
 * write it out when emptying the buffer externally.
 */

METHODDEF(boolean)
empty_output_buffer (j_compress_ptr cinfo)
{
	my_dest_ptr dest = (my_dest_ptr) cinfo->dest;

	//if (JFWRITE(dest->outstream, dest->buffer, OUTPUT_BUF_SIZE) !=
	//    (size_t) OUTPUT_BUF_SIZE)
	//  ERREXIT(cinfo, JERR_FILE_WRITE);
	JFWRITE(dest->outstream, dest->buffer, OUTPUT_BUF_SIZE);

	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

	return TRUE;
}


/*
 * Terminate destination --- called by jpeg_finish_compress
 * after all data has been written.  Usually needs to flush buffer.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF(void)
term_destination (j_compress_ptr cinfo)
{
	my_dest_ptr dest = (my_dest_ptr) cinfo->dest;
	size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;

	/* Write any data remaining in the buffer */
	if (datacount > 0) 
	{
		JFWRITE(dest->outstream, dest->buffer, static_cast<std::streamsize>(datacount));
	}
}


/*
 * Prepare for output to a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing compression.
 */

void imageboss_jpeg_stdio_dest (j_compress_ptr cinfo, std::ostream *outstream)
{
	my_dest_ptr dest;

	//cout << "In wallabie_jpeg_stdio_dest (ostream)" << endl;

	/* The destination object is made permanent so that multiple JPEG images
	* can be written to the same file without re-executing jpeg_stdio_dest.
	* This makes it dangerous to use this manager and a different destination
	* manager serially with the same JPEG object, because their private object
	* sizes may be different.  Caveat programmer.
	*/
	if (cinfo->dest == 0) 
	{
		/* first time for this JPEG object? */
		cinfo->dest = (struct jpeg_destination_mgr *)
			(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
			SIZEOF(my_destination_mgr));
	}

	dest = (my_dest_ptr) cinfo->dest;
	dest->pub.init_destination = init_destination;
	dest->pub.empty_output_buffer = empty_output_buffer;
	dest->pub.term_destination = term_destination;
	dest->outstream = outstream;

	return;
}


/* Expanded data source object for stdio input */

typedef struct 
{
	struct jpeg_source_mgr pub;	/* public fields */

	std::istream *instream;		/* source stream */
	JOCTET * buffer;		/* start of buffer */
	boolean start_of_file;	/* have we gotten any data yet? */
} my_source_mgr;

typedef my_source_mgr * my_src_ptr;

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

METHODDEF(void)
init_source (j_decompress_ptr cinfo)
{
	my_src_ptr src = (my_src_ptr) cinfo->src;

	/* We reset the empty-input-file flag for each image,
	* but we don't clear the input buffer.
	* This is correct behavior for reading a series of images from one source.
	*/
	src->start_of_file = TRUE;

	src->instream->seekg(0, std::ios::beg);
}


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr cinfo)
{
	my_src_ptr src = (my_src_ptr) cinfo->src;
	std::streamoff nbytes;
	std::streamoff diff;
	std::streampos posCurr;

	//nbytes = JFREAD(src->instream, src->buffer, INPUT_BUF_SIZE);
	//diff = src->instream->getLength() - src->instream->tellg();
	posCurr = src->instream->tellg();
	src->instream->seekg(0, std::ios::end);
	diff = src->instream->tellg() - posCurr;
	src->instream->seekg(posCurr, std::ios::beg);
	if(diff >= INPUT_BUF_SIZE)
	{
		JFREAD(src->instream, src->buffer, INPUT_BUF_SIZE);
		nbytes = INPUT_BUF_SIZE;
	}
	else
	{
		JFREAD(src->instream, src->buffer, static_cast<std::streamsize>(diff));
		nbytes = diff;
	}


	if (nbytes <= 0) 
	{
		if (src->start_of_file)	/* Treat empty input file as fatal error */
			ERREXIT(cinfo, JERR_INPUT_EMPTY);
	
		WARNMS(cinfo, JWRN_JPEG_EOF);
		/* Insert a fake EOI marker */
		src->buffer[0] = (JOCTET) 0xFF;
		src->buffer[1] = (JOCTET) JPEG_EOI;
		nbytes = 2;
	}

	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = static_cast<size_t>(nbytes);
	src->start_of_file = FALSE;

	return TRUE;
}


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

METHODDEF(void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
	my_src_ptr src = (my_src_ptr) cinfo->src;

	/* Just a dumb implementation for now.  Could use fseek() except
	* it doesn't work on pipes.  Not clear that being smart is worth
	* any trouble anyway --- large skips are infrequent.
	*/
	if (num_bytes > 0) 
	{
		while (num_bytes > (int) src->pub.bytes_in_buffer) 
		{
			num_bytes -= (int) src->pub.bytes_in_buffer;
			(void) fill_input_buffer(cinfo);
			/* note we assume that fill_input_buffer will never return FALSE,
			* so suspension need not be handled.
			*/
		}
		src->pub.next_input_byte += (size_t) num_bytes;
		src->pub.bytes_in_buffer -= (size_t) num_bytes;
	}
}


/*
 * An additional method that can be provided by data source modules is the
 * resync_to_restart method for error recovery in the presence of RST markers.
 * For the moment, this source module just uses the default resync method
 * provided by the JPEG library.  That method assumes that no backtracking
 * is possible.
 */


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF(void)
term_source (j_decompress_ptr cinfo)
{
	/* no work necessary here */
}


/*
 * Prepare for input from a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */

GLOBAL(void)
imageboss_jpeg_stdio_src (j_decompress_ptr cinfo, std::istream *instream)
{
	my_src_ptr src;

	/* The source object and input buffer are made permanent so that a series
	* of JPEG images can be read from the same file by calling jpeg_stdio_src
	* only before the first one.  (If we discarded the buffer at the end of
	* one image, we'd likely lose the start of the next one.)
	* This makes it unsafe to use this manager and a different source
	* manager serially with the same JPEG object.  Caveat programmer.
	*/
	if (cinfo->src == 0) 
	{	
		/* first time for this JPEG object? */
		cinfo->src = (struct jpeg_source_mgr *)
			(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
			SIZEOF(my_source_mgr));
		src = (my_src_ptr) cinfo->src;
		src->buffer = (JOCTET *)
			(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
			INPUT_BUF_SIZE * SIZEOF(JOCTET));
	}

	src = (my_src_ptr) cinfo->src;
	src->pub.init_source = init_source;
	src->pub.fill_input_buffer = fill_input_buffer;
	src->pub.skip_input_data = skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart;	/* use default method */
	src->pub.term_source = term_source;
	src->instream = instream;
	src->pub.bytes_in_buffer = 0;		/* forces fill_input_buffer on first read */
	src->pub.next_input_byte = 0;	/* until buffer loaded */

	return;
}
