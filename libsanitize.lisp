(in-package :common-lisp-user)

(defpackage :libsanitize
  (:use :common-lisp :cffi :trivial-garbage)
  (:export :load-sanitize-mode
	   :create-sanitize-mode
	   :sanitize))

(in-package :libsanitize)

(define-foreign-library libsanitize
  (:unix "libsanitize.so")
  (t (:default "libsanitize.so")))

(use-foreign-library libsanitize)

#+sbcl
(pushnew (lambda ()
           (use-foreign-library libsanitize))
         sb-ext:*init-hooks*)

(defcfun "mode_load" :pointer
  (path :string))

(defcfun "mode_memory" :pointer
  (mode-str :string))

(defcfun "mode_free" :void
  (mode :pointer))

(defcfun ("sanitize" %sanitize) :string
  (text :string)
  (mode :pointer))

(defstruct sanitize-mode
  ptr
  str)

(defmethod print-object ((obj sanitize-mode) stream)
  (print-unreadable-object (obj stream :type t :identity t)
    (princ (sanitize-mode-str obj) stream)))

(defun create-mode (ptr str)
  (unless (null-pointer-p ptr)
    (let ((mode (make-sanitize-mode :ptr ptr :str str)))
      (finalize mode (lambda ()
		       (mode-free ptr))))))

(defun load-sanitize-mode (path)
  (create-mode (mode-load (namestring path))
	       (namestring path)))

(defun create-sanitize-mode (mode-xml)
  (create-mode (mode-memory mode-xml)
	       mode-xml))

(defun sanitize (text mode)
  (%sanitize text (sanitize-mode-ptr mode)))

