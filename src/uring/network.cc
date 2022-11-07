  class ostream : public iofu::obstream_base<ostream,iovec> {
    public:
      using iofu::obstream_base<ostream,iovec>::operator<<;

      ostream(std::endian endian = std::endian::big,
              std::size_t slab_size = 32)
        : iofu::obstream_base<ostream,iovec>(endian, slab_size) {
        /* empty */
      }

      void operator||(std::function<void(void)> f) {
        // write();
      }
  };

  void loop::listen_tcp(uint16_t port, tcp_protocol::factory_t fac) {
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
      throw std::runtime_error("socket()");

    int enable = 1;
    if (setsockopt(sock,
                   SOL_SOCKET, SO_REUSEADDR,
                   &enable, sizeof(int)) < 0)
      throw std::runtime_error("setsockopt(SO_REUSEADDR)");

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0)
      throw std::runtime_error("bind()");

    if (listen(sock, 10) < 0)
      throw std::runtime_error("listen()");

    create_resource<tcp_acceptor>(sock, fac).accept();
  }

  auto loop::listen_tcp(uint16_t port) {
    return detail::callback_forward(
      [this, port](tcp_protocol::factory_t fac) { listen_tcp(port, fac); }
    );
  }

  class fd_resource : public resource {
    protected:
      int fd;

    public:
      fd_resource(loop&, int);
  };

  class tcp_connection : public fd_resource {
    protected:
      tcp_protocol* proto;

    public:
      tcp_connection(loop&, int, tcp_protocol::factory_t&);
      ~tcp_connection();

      void on_connect();

      void write(detail::ostream);

      detail::ostream write(std::endian endian = std::endian::big,
                            std::size_t slab_size = 32);

      template<typename T> detail::ostream operator<<(T);
      // template<typename T> detail::istream operator>>(T);
  };

  class tcp_acceptor : public fd_resource {
    protected:
      tcp_protocol::factory_t protocol_factory;
      sockaddr_in addr;
      socklen_t addr_len = sizeof(addr);
      char hbuf[NI_MAXHOST];
      char sbuf[NI_MAXSERV];

    public:
      tcp_acceptor(loop&, int, tcp_protocol::factory_t&);
      void on_accept(int);
      void accept();
  };

  // template <typename P>
  // void tcp_connection::write(std::string data, void (P::*func)()) {
  //   auto sqe = lp.create_sqe(
  //     // explicitly capture "data" so object doesn't go out of scope
  //     [this, func, data] (loop&, loop::res_t, loop::flags_t) {
  //       (reinterpret_cast<P*>(proto)->*func)();
  //     }
  //   );
  //   io_uring_prep_send(
  //     sqe, fd,
  //     data.c_str(),
  //     data.length(),
  //     0
  //   );
  //   lp.submit();
  // }

  detail::ostream tcp_connection::write(std::endian endian,
                                        std::size_t slab_size) {
    return detail::ostream(
      endian, slab_size// ,
      // []() {

      // }
    );
  }

  template<typename T>
  detail::ostream tcp_connection::operator<<(T value) {
    return write() << value;
  }

  tcp_acceptor::tcp_acceptor(loop& l, int f, tcp_protocol::factory_t& fac)
    : fd_resource(l, f), protocol_factory(fac) {
    /* empty */
  }

  void tcp_acceptor::on_accept(int fd) {
    std::cout << "on_accept" << std::endl;
    if (getnameinfo(reinterpret_cast<const sockaddr*>(&addr), addr_len,
                    hbuf, NI_MAXHOST, sbuf, NI_MAXSERV,
                    NI_NUMERICHOST | NI_NUMERICSERV) == 0)
      std::cout << "host=" << hbuf << ", serv=" << sbuf << std::endl;
    lp.create_resource<tcp_connection>(fd, protocol_factory).on_connect();
    accept();
  }

  void tcp_acceptor::accept() {
    auto sqe = lp.create_sqe([this] (loop&, __s32 r, __u32) { on_accept(r); });
    io_uring_prep_accept(
      sqe, fd,
      reinterpret_cast<sockaddr*>(&addr),
      &addr_len,
      0
    );
    lp.submit();
  }
