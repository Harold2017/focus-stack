#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
//#include <pybind11/functional.h>

#include "focusstack.hh"

namespace py = pybind11;
using namespace focusstack;

// code from https://github.com/ausk/keras-unet-deploy/blob/master/cpp/libunet/cvbind.h
namespace pybind11 {
    namespace detail {

        template<>
        struct type_caster<cv::Mat> {
        public:

            PYBIND11_TYPE_CASTER(cv::Mat, _("numpy.ndarray"));

            //! 1. cast numpy.ndarray to cv::Mat    
            bool load(handle obj, bool) {
                array b = reinterpret_borrow<array>(obj);
                buffer_info info = b.request();

                int nh = 1;
                int nw = 1;
                int nc = 1;
                int ndims = info.ndim;
                if (ndims == 2) {
                    nh = info.shape[0];
                    nw = info.shape[1];
                }
                else if (ndims == 3) {
                    nh = info.shape[0];
                    nw = info.shape[1];
                    nc = info.shape[2];
                }
                else {
                    throw std::logic_error("Only support 2d, 2d matrix");
                    return false;
                }

                int dtype;
                if (info.format == format_descriptor<unsigned char>::format()) {
                    dtype = CV_8UC(nc);
                }
                else if (info.format == format_descriptor<int>::format()) {
                    dtype = CV_32SC(nc);
                }
                else if (info.format == format_descriptor<float>::format()) {
                    dtype = CV_32FC(nc);
                }
                else {
                    throw std::logic_error("Unsupported type, only support uchar, int32, float");
                    return false;
                }
                value = cv::Mat(nh, nw, dtype, info.ptr);
                return true;
            }

            //! 2. cast cv::Mat to numpy.ndarray    
            static handle cast(const cv::Mat& mat, return_value_policy, handle defval) {
                std::string format = format_descriptor<unsigned char>::format();
                size_t elemsize = sizeof(unsigned char);
                int nw = mat.cols;
                int nh = mat.rows;
                int nc = mat.channels();
                int depth = mat.depth();
                int type = mat.type();
                int dim = (depth == type) ? 2 : 3;
                if (depth == CV_8U) {
                    format = format_descriptor<unsigned char>::format();
                    elemsize = sizeof(unsigned char);
                }
                else if (depth == CV_32S) {
                    format = format_descriptor<int>::format();
                    elemsize = sizeof(int);
                }
                else if (depth == CV_32F) {
                    format = format_descriptor<float>::format();
                    elemsize = sizeof(float);
                }
                else {
                    throw std::logic_error("Unsupport type, only support uchar, int32, float");
                }

                std::vector<size_t> bufferdim;
                std::vector<size_t> strides;
                if (dim == 2) {
                    bufferdim = { (size_t)nh, (size_t)nw };
                    strides = { elemsize * (size_t)nw, elemsize };
                }
                else if (dim == 3) {
                    bufferdim = { (size_t)nh, (size_t)nw, (size_t)nc };
                    strides = { (size_t)elemsize * nw * nc, (size_t)elemsize * nc, (size_t)elemsize };
                }
                return array(buffer_info(mat.data, elemsize, format, dim, bufferdim, strides)).release();
            }
        };

    }
}// namespace pybind11::detail

PYBIND11_MODULE(focus_stack_py, m)
{
    m.doc() = "This currently is py wrapper for Focus Stack Blocking Interface Only";

    py::enum_<FocusStack::align_flags_t>(m.def_submodule("FocusStackAlign", "align flags enumerator"), "AlignFlags")
        .value("ALIGN_DEFAULT", FocusStack::ALIGN_DEFAULT)
        .value("ALIGN_NO_WHITEBALANCE", FocusStack::ALIGN_NO_WHITEBALANCE)
        .value("ALIGN_NO_CONTRAST", FocusStack::ALIGN_NO_CONTRAST)
        .value("ALIGN_FULL_RESOLUTION", FocusStack::ALIGN_FULL_RESOLUTION)
        .value("ALIGN_GLOBAL", FocusStack::ALIGN_GLOBAL)
        .value("ALIGN_KEEP_SIZE", FocusStack::ALIGN_KEEP_SIZE)
        .export_values();

    py::enum_<FocusStack::log_level_t>(m.def_submodule("FocusStackLogLevel", "log level enumerator"), "LogLevel")
        .value("LOG_VERBOSE", FocusStack::LOG_VERBOSE)
        .value("LOG_PROGRESS", FocusStack::LOG_PROGRESS)
        .value("LOG_INFO", FocusStack::LOG_INFO)
        .value("LOG_ERROR", FocusStack::LOG_ERROR)
        .export_values();

    py::class_<FocusStack>(m, "FocusStack").def(py::init<>())
        .def("set_inputs", &FocusStack::set_inputs)
        .def("set_output", &FocusStack::set_output, py::arg("output")="output.jpg")
        .def("get_output", &FocusStack::get_output)
        .def("set_depthmap", &FocusStack::set_depthmap, py::arg("depthmap")="depthmap.png")
        .def("get_depthmap", &FocusStack::get_depthmap)
        .def("set_3dview", &FocusStack::set_3dview, py::arg("filename_3dview")="3dview.png")
        .def("get_3dview", &FocusStack::get_3dview)
        .def("set_depthmap_threshold", &FocusStack::set_depthmap_threshold)
        .def("set_depthmap_smooth_xy", &FocusStack::set_depthmap_smooth_xy)
        .def("set_depthmap_smooth_z", &FocusStack::set_depthmap_smooth_z)
        .def("set_halo_radius", &FocusStack::set_halo_radius)
        .def("set_remove_bg", &FocusStack::set_remove_bg)
        .def("set_disable_opencl", &FocusStack::set_disable_opencl)
        .def("set_save_steps", &FocusStack::set_save_steps)
        .def("set_nocrop", &FocusStack::set_nocrop)
        .def("set_align_only", &FocusStack::set_align_only)
        .def("set_verbose", &FocusStack::set_verbose)
        .def("set_threads", &FocusStack::set_threads)
        .def("set_batchsize", &FocusStack::set_batchsize)
        .def("set_reference", &FocusStack::set_reference)
        .def("set_jpgquality", &FocusStack::set_jpgquality)
        .def("set_consistency", &FocusStack::set_consistency)
        .def("set_denoise", &FocusStack::set_denoise)
        .def("set_wait_images", &FocusStack::set_wait_images)
        .def("set_align_flags", &FocusStack::set_align_flags)
        .def("set_3dviewpoint", static_cast<void (FocusStack::*)(float, float, float, float)>(&FocusStack::set_3dviewpoint))
        .def("set_3dviewpoint", static_cast<void (FocusStack::*)(std::string)>(&FocusStack::set_3dviewpoint))
        // FIXME: use py function as log callback will hang after first call even do nothing
        //.def("set_log_callback", &FocusStack::set_log_callback)
        .def("run", &FocusStack::run)
        //.def("start", &FocusStack::start)
        //.def("add_image", static_cast<void (FocusStack::*)(std::string)>(&FocusStack::add_image))
        //.def("add_image", static_cast<void (FocusStack::*)(const cv::Mat&)>(&FocusStack::add_image))
        //.def("do_final_merge", &FocusStack::do_final_merge)
        //// https://github.com/pybind/pybind11/issues/134#issuecomment-1853997216
        // TODO: how to pass argument as reference to class member function? maybe use global object?
        //.def("get_status", &FocusStack::get_status)
        //.def("wait_done", &FocusStack::wait_done, py::arg("status") = [](bool status) {&FocusStack::wait_done(status); return status; }, py::arg("errmsg"), py::kw_only(), py::arg("timeout_ms") = -1)
        //.def("reset", &FocusStack::reset, py::arg("keep_results")=false)
        .def("get_result_image", &FocusStack::get_result_image)
        .def("get_result_depthmap", &FocusStack::get_result_depthmap)
        .def("get_result_mask", &FocusStack::get_result_mask)
        .def("get_result_3dview", &FocusStack::get_result_3dview)
        .def("regenerate_depthmap", &FocusStack::regenerate_depthmap)
        .def("regenerate_mask", &FocusStack::regenerate_mask)
        .def("regenerate_3dview", &FocusStack::regenerate_3dview);
}