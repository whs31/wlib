#include <w/common/bytes.h>

W_COMMON_NAMESPACE_BEGIN

ByteReader::ByteReader(std::span<byte const> const data) noexcept
  : m_data(data)
  , m_size(data.size())
{}

std::span<byte const> ByteReader::take(usize const count)
{
  if(count > this->m_data.size())
    throw ByteReadError("input ends early: {} bytes needed, {} left", count, this->m_data.size());

  auto const head = this->m_data.first(count);
  this->m_data = this->m_data.subspan(count);
  return head;
}

void ByteReader::expect_end() const
{
  if(not this->m_data.empty())
    throw ByteReadError("input has {} trailing bytes", this->m_data.size());
}

usize ByteReader::remaining() const noexcept { return this->m_data.size(); }

ByteWriter& ByteWriter::append(std::span<byte const> const bytes)
{
  this->m_data.insert(this->m_data.end(), bytes.begin(), bytes.end());
  return *this;
}

std::vector<byte> ByteWriter::finish() && noexcept { return std::move(this->m_data); }

W_COMMON_NAMESPACE_END
